/*This module executes all the instructions in PC*/
#include "emulator.h"
enum ALU{buffer, ADD, ADDC, SUB, SUBC, CMP, XOR, AND, OR};
enum CEX_CCC{EQ, NE, CS, CC, MI, PL, VS, VC, HI, LS, GE, LT, GT, LE, TR, FL};
#define NEGSIGN(x)	(0xFFFF << (x))

cex CEX;
extern psw PSW;

/*Calculates the sign extension for an offset and returns a sign extended offset*/
unsigned short sign_ext(unsigned short offset, unsigned short signbit)
{
/* Sign extend at signbit
 - return sign-extended value
*/
    if (offset & (1 << signbit))
        offset |= NEGSIGN(signbit); /* bit set - negative */
/* bit clear - positive */
    return offset;
}

/*jumps to a new branch based on the offset 13 bit offset*/
void execute_BL(unsigned short instruction){
    //create an effective address
    unsigned short offset;
    //assign the program counter to the link register
    LR.word = PC.word;
    //find the effective address using sign extension(thanks to Dr. Larry Hughes for providing the function for this)
    //add the sign extended value to the PC so it can be offseted to the instruction to be executed next
    offset = BLOFFSET(instruction);
    PC.word += sign_ext(offset, 12) << 1;
    //turn off the cex state when BL encountered
    CEX.STATE = OFF;
}

/*jumps to a new branch based on the offset 10 bit offset*/
void execute_BR(unsigned short instruction){
    //create an effective address
    unsigned short offset;
    offset = BROFFSET(instruction);
    PC.word += sign_ext(offset, 9) << 1;
    //turn off the cex state when BL encountered
    CEX.STATE = OFF;
}

/*executes load with relative addressing*/
void execute_LDR(unsigned short instruction){

    unsigned short DRA_BIT, offset, WB_BIT, address, dest;

    //get the DRA bit first it is the 12th bit
    DRA_BIT = EXTRACT_BIT(instruction, 12);

    //get the offset
    offset = LDROFFSET(instruction);
    offset = sign_ext(offset, 4);

    //determine if word or byte it is the 6th bit
    WB_BIT = EXTRACT_BIT(instruction, 6);

    //extract the address
    address = BIT3TO5(instruction);

    //extract the destination register
    dest = BIT0TO2(instruction);

    //calculate the effective address
    mar = regfile[REG][(address | 0x08)].word + offset;

    // for a word
    if (WB_BIT == word) {
        bus(mar, &mbr, read, word);
        regfile[REG][dest | DRA_BIT << 3].word = mbr;
    }

    //for a byte
    else {
        bus(mar, &mbr, read, byte);
        regfile[REG][dest | DRA_BIT << 3].byte[0] = mbr;

    }
}

/*executes store with relative addressing*/
void execute_STR(unsigned short instruction){
    unsigned SRA_BIT, offset, WB_BIT, source, address;

    //get the DRA bit first it is the 12th bit
    SRA_BIT = EXTRACT_BIT(instruction, 12);

    //get the offset
    offset = LDROFFSET(instruction);
    offset = sign_ext(offset, 4);

    //determine if word or byte it is the 6th bit
    WB_BIT = EXTRACT_BIT(instruction, 6);

    //extract the address
    source = BIT3TO5(instruction);

    //extract the destination register
    address = BIT0TO2(instruction);

    //calculate the effective address
    mar = regfile[REG][(address | 0x08)].word + offset;
    // for a word
    if (WB_BIT == word) {
        mbr = regfile[REG][source | SRA_BIT << 3].word;
        bus(mar, &mbr, write, word);
    }

    //for a byte
    else {
        mbr = regfile[REG][source | SRA_BIT << 3].byte[0];
        bus(mar, &mbr, write, byte);
    }
}

/*executes load and store without relative addressing*/
void execute_LD_ST(unsigned  short instruction){
    //bit 11 differentiates if it is LD or ST
    unsigned short bit11, DI, DRA, SRA, PRPO, ID, WB, address_LD, address_ST, dest, source;
    //bit 11 for LD or ST
    bit11 = EXTRACT_BIT(instruction, 11);
    //direct or indexed
    DI = EXTRACT_BIT(instruction, 10);
    //destination register is either data or address register
    DRA = EXTRACT_BIT(instruction, 9);
    //destination register is either data or address register
    SRA = EXTRACT_BIT(instruction, 9);
    //Pre/Post Address register increment/decrement
    PRPO = EXTRACT_BIT(instruction, 8);
    //Increment or decrement
    ID = EXTRACT_BIT(instruction, 7);
    //word or a byte
    WB = EXTRACT_BIT(instruction, 6);
    //address for load
    address_LD = BIT3TO5(instruction);
    //address for store
    address_ST = BIT0TO2(instruction);
    //destination register
    dest = BIT0TO2(instruction);
    //source register
    source = BIT3TO5(instruction);


    if (bit11 == 0){
        /* -------------------------LD------------------------------*/
        //direct
        if(DI == 0){
            mar = regfile[REG][address_LD | 0x08].word;
        }
        //indexed
        else{ // do pre- increment/decrement
            if(PRPO == 0){ //Pre-incr/decr
                if(ID == 0){ //Increment
                    regfile[REG][address_LD | 0x08].word  = (WB == 0) ? regfile[REG][address_LD | 0x08].word + 2 : regfile[REG][address_LD | 0x08].word + 1;
                }
                else{ //Decrement
                    regfile[REG][address_LD | 0x08].word  = (WB == 0) ? regfile[REG][address_LD | 0x08].word - 2 : regfile[REG][address_LD | 0x08].word - 1;
                }
            }
            mar = regfile[REG][address_LD | 0x08].word;
        }

        // do the Load here
        if(WB == 0){ // this a word
            bus(mar, &mbr, read, word);
            regfile[REG][dest | DRA << 3].word = mbr;
        }
        else{ // this is a byte
            bus(mar, &mbr, read, byte);
            regfile[REG][dest | DRA << 3].byte[0] = mbr;
        }

        //do post increment/decrement here
        if(DI == 1 && PRPO == 1){
            //Indexed post-incr/decr
            if(ID == 0){//increment
                regfile[REG][address_LD | 0x08].word  = (WB == 0) ? regfile[REG][address_LD | 0x08].word + 2 : regfile[REG][address_LD | 0x08].word + 1;
            }
            else{//decrement
                regfile[REG][address_LD | 0x08].word  = (WB == 0) ? regfile[REG][address_LD | 0x08].word-2 : regfile[REG][address_LD | 0x08].word - 1;
            }
        }
    }

    else{
        /* -------------------------ST------------------------------*/
        //direct
        if(DI == 0){
            mar = regfile[REG][address_ST | 0x08].word;
        }
        //indexed
        else{ // pre- increment/decrement
            if(PRPO == 0){ //Pre-incr/decr
                if(ID == 0){ //Increment
                    regfile[REG][address_ST | 0x08].word  = (WB == 0) ? regfile[REG][address_ST | 0x08].word + 2 : regfile[REG][address_ST | 0x08].word + 1;
                }
                else{ //Decrement
                    regfile[REG][address_ST | 0x08].word  = (WB == 0) ? regfile[REG][address_ST | 0x08].word - 2 : regfile[REG][address_ST | 0x08].word - 1;
                }
            }
            mar = regfile[REG][address_ST | 0x08].word;
        }

        // Do the ST here
        if(WB == 0){ // this a word
            mbr = regfile[REG][source | SRA << 3].word;
            bus(mar, &mbr, write, word);
        }
        else{ // this is a byte
            mbr = regfile[REG][source | SRA << 3].byte[0];
            bus(mar, &mbr, write, byte);
        }

        if(DI == 1 && PRPO == 1){
            //Indexed post-incr/decr
            if(ID == 0){//increment
                regfile[REG][address_ST | 0x08].word  = (WB == 0) ? regfile[REG][address_ST | 0x08].word + 2 : regfile[REG][address_ST | 0x08].word + 1;
            }
            else{//decrement
                regfile[REG][address_ST | 0x08].word  = (WB == 0) ? regfile[REG][address_ST | 0x08].word - 2 : regfile[REG][address_ST | 0x08].word - 1;
            }
        }

    }
}

/*Executes MOV*/
void execute_MOV(unsigned short instruction){
    unsigned short SRA, DRA, WB, source, dest;
    //the SRA bit is the 8th bit
    SRA = EXTRACT_BIT(instruction, 8);
    //the DRA bit is the 7th bit
    DRA = EXTRACT_BIT(instruction, 7);
    //the word/byte bit
    WB = EXTRACT_BIT(instruction, 6);
    //source register
    source = BIT3TO5(instruction);
    //the destination register
    dest = BIT0TO2(instruction);

    if(WB == word) regfile[REG][dest | DRA << 3].word = regfile[REG][source | SRA << 3].word;
    else regfile[REG][dest | DRA << 3].byte[0] = regfile[REG][source | SRA << 3].byte[0];

}

/*Swaps 2 words*/
void execute_SWAP(unsigned short instruction){
    unsigned short SRA, DRA, source, dest;
    union word_byte temp;

    SRA = EXTRACT_BIT(instruction, 8);
    DRA = EXTRACT_BIT(instruction, 7);
    source = BIT3TO5(instruction);
    dest = BIT0TO2(instruction);

    temp.word = regfile[REG][dest | DRA << 3].word;
    regfile[REG][dest | DRA << 3].word = regfile[REG][source | SRA << 3].word;
    regfile[REG][source | SRA << 3].word = temp.word;
}

/*shifts right arthematic*/
void execute_SRA(unsigned short instruction){

    unsigned short WB, dest;
    union word_byte temp;
    WB = EXTRACT_BIT(instruction, 6);
    dest = BIT0TO2(instruction);


    //PSW.C = EXTRACT_BIT(regfile[0][dest].word, 0);
    PSW.C = (regfile[REG][dest].word & 0x01);

    if(WB == word){
        //get the signed bit
        temp.word = EXTRACT_BIT(regfile[REG][dest].word, 15);
        regfile[REG][dest].word = (regfile[REG][dest].word >> 1) | (temp.word << 15);
    }
    else{
        //get the signed bit
        temp.byte[REG] = EXTRACT_BIT(instruction, 7);
        regfile[REG][dest].byte[0] = (regfile[REG][dest].byte[0] >> 1) | (temp.byte[0] << 7);
    }
}

/*Rotate Right through carry*/
void execute_RRC(unsigned short instruction){

    unsigned  short temp, WB,dest;
    dest = BIT0TO2(instruction);
    temp = PSW.C;
    PSW.C = EXTRACT_BIT(regfile[0][dest].word, 0);
    WB = EXTRACT_BIT(instruction, 6);

    if(WB == word){
        regfile[REG][dest].word = (regfile[REG][dest].word >> 1) | (temp << 15);
    }
    else{
        regfile[REG][dest].byte[0] = (regfile[REG][dest].byte[0] >> 1) | (temp << 7);
    }
}

/*implements sign extension*/
void execute_SXT(unsigned short instruction){

    unsigned short dst;
    dst = BIT0TO2(instruction);
    regfile[REG][dst].word = sign_ext(regfile[0][dst].word, 7);
}

/*Swap bytes in register*/
void execute_SWBP(unsigned short instruction){

    unsigned short temp, dest;
    dest = BIT0TO2(instruction);
    temp = regfile[0][dest].byte[0];
    regfile[0][dest].byte[0] = regfile[0][dest].byte[1];
    regfile[0][dest].byte[1] = temp;
}

/*Bit test*/
void execute_BIT(unsigned short instruction){

    unsigned short RC, WB, source_constant, dest;
    RC = EXTRACT_BIT(instruction, 7);
    WB = EXTRACT_BIT(instruction, 6);
    source_constant = BIT3TO5(instruction);
    dest = BIT0TO2(instruction);

    if(WB == word){
        if(((regfile[0][dest].word >> regfile[RC][source_constant].word) & 0x01) == 0){
            PSW.Z = 1;
        }
    }
    else{
        if(((regfile[0][dest].byte[0] >> regfile[RC][source_constant].byte[0]) & 0x01) == 0){
            PSW.Z = 1;
        }
    }
}

/*Bit Set*/
void execute_BIS(unsigned short instruction){

    union word_byte result;
    unsigned short  RC, WB, source_constant, dest;
    RC = EXTRACT_BIT(instruction, 7);
    WB = EXTRACT_BIT(instruction, 6);
    source_constant = BIT3TO5(instruction);
    dest = BIT0TO2(instruction);

    if(WB == word){
        result.word = regfile[0][dest].word | (1 << regfile[RC][source_constant].word);
        update_psw_logic(result.word,0);
        regfile[0][dest].word = result.word;
    }
    else{
        result.byte[0] = regfile[0][dest].byte[0] | (1 << regfile[RC][source_constant].byte[0]);
        update_psw_logic( result.byte[0],1);
        regfile[0][dest].byte[0] = result.byte[0];
    }
}

/*Bit Clear*/
void execute_BIC(unsigned short instruction){

    unsigned short RC, WB, source_constant, dest;
    RC = EXTRACT_BIT(instruction, 7);
    WB = EXTRACT_BIT(instruction, 6);
    source_constant = BIT3TO5(instruction);
    dest = BIT0TO2(instruction);
    union word_byte result;

    if (WB == word){
        result.word = regfile[0][dest].word & (~(1<<regfile[RC][source_constant].word));
        update_psw_logic(result.word, 0);
        regfile[0][dest].word = result.word;
    }
    else{
        result.word = regfile[0][dest].byte[0] & (~(1<<regfile[RC][source_constant].byte[0]));
        update_psw_logic( result.byte[0],1);
        regfile[0][dest].byte[0] = result.byte[0];
    }
}

/*Moves a byte into LSByte of the register */
void execute_MOVL(unsigned short instruction){\

    unsigned short DRA, dest;
    DRA = EXTRACT_BIT(instruction, 11);
    dest = BIT0TO2(instruction) | DRA << 3;

    regfile[REG][dest].byte[0] = MOVX_DATA(instruction);
}

/*Moves a byte into LSByte of the register and sets MSByte to 0*/
void execute_MOVLZ(unsigned int instruction){

    unsigned short DRA, dest;
    DRA = EXTRACT_BIT(instruction, 11);
    dest = BIT0TO2(instruction) | DRA << 3;
    regfile[REG][dest].byte[0] = MOVX_DATA(instruction);
    regfile[REG][dest].byte[1] = 0b00000000;
}

/*Moves a byte into LSByte of the register and sets MSByte to 1*/
void execute_MOVLS(unsigned short instruction){

    unsigned short DRA, dest;
    DRA = EXTRACT_BIT(instruction, 11);
    dest = BIT0TO2(instruction) | DRA << 3;
    regfile[0][dest].byte[0] = MOVX_DATA(instruction);
    regfile[0][dest].byte[1] = 0b11111111;
}

/*Moves a byte into MSByte of the register*/
void execute_MOVLH(unsigned short instruction){

    unsigned short DRA, dest;
    DRA = EXTRACT_BIT(instruction, 11);
    dest = BIT0TO2(instruction) | DRA << 3;
    regfile[REG][dest].byte[1] = MOVX_DATA(instruction);
}

/*Executes conditional execution*/
void execute_CEX(unsigned short instruction){\

    unsigned short code;
    code = CEX_CODE(instruction);
    switch(code){
        case EQ:
            if(PSW.Z == 1) CEX.STATE = TRUE;
            else CEX.STATE = FALSE;
            break;
        case NE:
            if(PSW.Z == 0) CEX.STATE = TRUE;
            else CEX.STATE = FALSE;
            break;
        case CS:
            if(PSW.C == 1) CEX.STATE = TRUE;
            else CEX.STATE = FALSE;
            break;
        case CC:
            if(PSW.C == 0) CEX.STATE = TRUE;
            else CEX.STATE = FALSE;
            break;
        case MI:
            if(PSW.N == 1) CEX.STATE = TRUE;
            else CEX.STATE = FALSE;
            break;
        case PL:
            if(PSW.N == 0) CEX.STATE = TRUE;
            else CEX.STATE = FALSE;
            break;
        case VS:
            if(PSW.V == 1) CEX.STATE = TRUE;
            else CEX.STATE = FALSE;
            break;
        case VC:
            if(PSW.V == 0) CEX.STATE = TRUE;
            else CEX.STATE = FALSE;
            break;
        case HI:
            if(PSW.C == 1 && PSW.Z == 0) CEX.STATE = TRUE;
            else CEX.STATE = FALSE;
            break;
        case LS:
            if(PSW.C == 0 && PSW.Z == 1) CEX.STATE = TRUE;
            else CEX.STATE = FALSE;
            break;
        case GE:
            if(PSW.N == PSW.V) CEX.STATE = TRUE;
            else CEX.STATE = FALSE;
            break;
        case LT:
            if(PSW.N != PSW.V) CEX.STATE = TRUE;
            else CEX.STATE = FALSE;
            break;
        case GT:
            if(PSW.Z == 0 && (PSW.N == PSW.V)) CEX.STATE = TRUE;
            else CEX.STATE = FALSE;
            break;
        case LE:
            if(PSW.Z == 0 && (PSW.N != PSW.V)) CEX.STATE = TRUE;
            else CEX.STATE = FALSE;
            break;
        case TR:
            CEX.STATE = TRUE;
            break;
        case FL:
            CEX.STATE = FALSE;
            break;
    }
    CEX.FFF = BIT0TO2(instruction);
    CEX.TTT = BIT3TO5(instruction);
}

/*Executes ADD to OR instructions*/
void execute_ADDtoOR(unsigned short instruction){
    unsigned short RC, WB, source_constant, dest;
    union word_byte result;

    RC = EXTRACT_BIT(instruction, 7);
    WB = EXTRACT_BIT(instruction, 6);
    source_constant = BIT3TO5(instruction);
    dest = BIT0TO2(instruction);

    switch(BIT8TO11(instruction)){
        case ADD:
            if (WB == word) {
                result.word = regfile[REG][dest].word + regfile[RC][source_constant].word;
                update_psw(regfile[RC][source_constant].word, regfile[REG][dest].word, result.word, 0);
                regfile[REG][dest].word =result.word;
            }
            else {
                result.byte[0] = regfile[REG][dest].byte[0] + regfile[RC][source_constant].byte[0];
                update_psw(regfile[RC][source_constant].byte[0], regfile[REG][dest].byte[0], result.byte[0], 1);
                regfile[REG][dest].byte[0] = result.byte[0];
            }
            break;
        case ADDC:
            if (WB == word) {
                result.word  = regfile[REG][dest].word + regfile[RC][source_constant].word + PSW.C;
                update_psw(regfile[RC][source_constant].word, regfile[REG][dest].word, result.word, 0);
                regfile[0][dest].word = result.word;
            }
            else {
                result.byte[0] = regfile[REG][dest].byte[0] + regfile[RC][source_constant].byte[0] + PSW.C;
                update_psw(regfile[RC][source_constant].byte[0], regfile[REG][dest].byte[0], result.byte[0], 1);
                regfile[REG][dest].byte[0]= result.byte[0];
            }
            break;
        case SUB:
            if (WB == word) {
                result.word = regfile[REG][dest].word + (~(regfile[RC][source_constant].word)) + 1;
                update_psw(~(regfile[RC][source_constant].word), regfile[REG][dest].word , result.word, 0);
                regfile[REG][dest].word = result.word;
            }
            else {
                result.byte[0] = regfile[REG][dest].byte[0] + (~(regfile[RC][source_constant].byte[0]))+ 1;
                update_psw(~(regfile[RC][source_constant].byte[0]), regfile[REG][dest].byte[0], result.byte[0], 1);
                regfile[REG][dest].byte[0] = result.byte[0];
            }
            break;
        case SUBC:
            if (WB == word) {
                result.word = regfile[REG][dest].word + ~(regfile[RC][source_constant].word) + PSW.C;
                update_psw(~(regfile[RC][source_constant].word), regfile[REG][dest].word, result.word, 0);
                regfile[REG][dest].word = result.word;
            }
            else {
                result.byte[0] = regfile[0][dest].byte[0] + ~(regfile[RC][source_constant].byte[0]) + PSW.C;
                update_psw(~(regfile[RC][source_constant].byte[0]), regfile[REG][dest].byte[0], result.byte[0], 1);
                regfile[REG][dest].byte[0] = result.byte[0];
            }
            break;
        case CMP:
            if (WB == word) {
                result.word = regfile[REG][dest].word + ~(regfile[RC][source_constant].word) + 1;
                update_psw( ~(regfile[RC][source_constant].word), regfile[REG][dest].word, result.word, 0);
            }
            else {
                result.byte[0] = regfile[REG][dest].byte[0] + ~(regfile[RC][source_constant].byte[0]) + 1;
                update_psw(~(regfile[RC][source_constant].byte[0]), regfile[REG][dest].byte[0], result.byte[0], 1);
            }
            break;
        case XOR:
            if (WB == word) {
                result.word = regfile[REG][dest].word ^ regfile[RC][source_constant].word;
                update_psw(regfile[RC][source_constant].word, regfile[REG][dest].word, result.word, 0);
                regfile[0][dest].word = result.word;
            }
            else {
                result.byte[0] = regfile[0][dest].byte[0] ^ regfile[RC][source_constant].byte[0];
                update_psw(regfile[RC][source_constant].byte[0], regfile[REG][dest].byte[0], result.byte[0], 1);
                regfile[REG][dest].byte[0] = result.byte[0];
            }
            break;
        case AND:
            if (WB == word) {
                result.word = regfile[REG][dest].word & regfile[RC][source_constant].word;
                update_psw(regfile[RC][source_constant].word, regfile[REG][dest].word, result.word, 0);
                regfile[REG][dest].word = result.word;
            }
            else {
                result.byte[0] = regfile[REG][dest].byte[0] & regfile[RC][source_constant].byte[0];
                update_psw(regfile[RC][source_constant].byte[0], regfile[REG][dest].byte[0], result.byte[0], 1);
                regfile[REG][dest].byte[0] = result.byte[0];
            }
            break;
        case OR:
            if (WB == word) {
                result.word = regfile[REG][dest].word | regfile[RC][source_constant].word;
                update_psw(regfile[RC][source_constant].word, regfile[REG][dest].word, result.word, 0);
                regfile[REG][dest].word = result.word;
            }
            else {
                result.byte[0] = regfile[REG][dest].byte[0] | regfile[RC][source_constant].byte[0];
                update_psw(regfile[RC][source_constant].byte[0], regfile[0][dest].byte[0], result.byte[0], 1);
                regfile[REG][dest].byte[0] = result.byte[0];
            }
            break;
    }
}


void execute_addressinginst(unsigned short instruction){
    enum {ADDA, SUBA, CMPA};
    unsigned short DRA, SRA, dest, source, WB, RC, opcode;

    union word_byte result;

    /*extract the DRA bit*/
    DRA = EXTRACT_BIT(instruction, 8);
    WB = EXTRACT_BIT(instruction, 12);
    SRA = EXTRACT_BIT(instruction, 7);
    source = BIT3TO5(instruction);
    dest = BIT0TO2(instruction);
    RC = EXTRACT_BIT(instruction, 6);
    opcode = BIT9TO11(instruction);

    switch (opcode) {

        case ADDA:
            if (WB == word) {

                result.word = regfile[REG][dest | DRA << 3].word + regfile[RC][source | SRA << 3].word;
                update_psw(regfile[RC][source | SRA << 3].word, regfile[REG][dest | DRA << 3].word, result.word, 0);
                regfile[REG][dest | DRA << 3].word =result.word;
            }
            else {

                result.byte[0] = regfile[REG][dest | DRA << 3].byte[0] + regfile[RC][source | SRA << 3].byte[0];
                update_psw(regfile[RC][source | SRA << 3].byte[0], regfile[REG][dest | DRA << 3].byte[0], result.byte[0], 1);
                regfile[REG][dest | DRA << 3].byte[0] = result.byte[0];
            }
            break;
        case SUBA:
            if (WB == word) {

                result.word = regfile[REG][dest | DRA << 3].word + (~(regfile[RC][source | SRA << 3].word)) + 1;
                update_psw(~(regfile[RC][source | SRA << 3].word), regfile[REG][dest | DRA << 3].word , result.word, 0);
                regfile[REG][dest | DRA << 3].word = result.word;
            }
            else {

                result.byte[0] = regfile[REG][dest | DRA << 3].byte[0] + (~(regfile[RC][source | SRA << 3].byte[0]))+ 1;
                update_psw(~(regfile[RC][source | SRA << 3].byte[0]), regfile[REG][dest | DRA << 3].byte[0], result.byte[0], 1);
                regfile[REG][dest | DRA << 3].byte[0] = result.byte[0];
            }
            break;
        case CMPA:
            if (WB == word) {
                result.word = regfile[REG][dest | DRA << 3].word + ~(regfile[RC][source | SRA << 3].word) + 1;
                update_psw( ~(regfile[RC][source | SRA << 3].word), regfile[REG][dest | DRA << 3].word, result.word, 0);
            }
            else {
                result.byte[0] = regfile[REG][dest | DRA << 3].byte[0] + ~(regfile[RC][source | SRA << 3].byte[0]) + 1;
                update_psw(~(regfile[RC][source | SRA << 3].byte[0]), regfile[REG][dest | DRA << 3].byte[0], result.byte[0], 1);
            }
            break;
    }
}

/*The execute function calls multiple functions to execute different instructions*/
void execute(enum INSTRUCTION inst) {
    CLOCK++;
    unsigned short instruction, BIT12, BIT10TO12;
    // instruction acts like a buffer register to store the instruction
    instruction = mbr;
    // inst is an enum variable attained from the decoder
    switch (inst) {
        case BL:
            execute_BL(instruction);
            break;
        case BR_TO_SWAP:
            //further decode
            BIT10TO12 = BIT10TO12(instruction);
            if(BIT10TO12 == 0){
                //BR
                execute_BR(instruction);
            }
            else if(BIT10TO12 == 1){
                //CEX
                execute_CEX(instruction);
            }
            else if(BIT10TO12 == 2){
                if(EXTRACT_BIT(instruction, 6) == 1){
                    //setcc
                    //clrcc
                }
                else{
                    if(EXTRACT_BIT(instruction, 5) == 1){
                        //SVC
                    }
                    else{
                        if(BIT3TO4(instruction) == 0){
                            //SWBP
                        }
                        else if(BIT3TO4(instruction) == 1){
                            //SXT
                        }
                        else{
                            //SETPRI
                        }
                    }
                }
            }
            else if(BIT10TO12 == 4){
                //SRA TO SUB
                if(EXTRACT_BIT(instruction, 9) == 1) execute_ADDtoOR(instruction);//ADDC AND SUB
                else{
                    if(EXTRACT_BIT(instruction, 8) == 1) execute_ADDtoOR(instruction);//ADD
                    else{
                        if(EXTRACT_BIT(instruction, 7) == 0) {
                            execute_SRA(instruction);//SRA
                        }
                        else execute_RRC(instruction);//RRC
                    }
                }
            }
            else if(BIT10TO12 == 5){
                //SUBC to AND
                execute_ADDtoOR(instruction);
            }
            else if(BIT10TO12 == 6){
                //OR TO BIC
                switch (BIT8TO10(instruction)) {
                    //OR
                    case 0:
                        execute_ADDtoOR(instruction);
                        break;
                    //BIT
                    case 1:
                        execute_BIT(instruction);
                        break;
                    //BIS
                    case 2:
                        execute_BIS(instruction);
                        break;
                    //BIC
                    case 3:
                        execute_BIC(instruction);
                        break;
                }
            }
            else if(BIT10TO12 == 7){
                if (EXTRACT_BIT(instruction, 9) == 0) execute_MOV(instruction);
                else execute_SWAP(instruction);
            }
            break;
        case addressing_instructions:
            execute_addressinginst(instruction);
            break;
        case LD_ST:
            //further decode
            execute_LD_ST(instruction);
            break;
        case MOVL_MOVLZ:
            BIT12 = EXTRACT_BIT(instruction, 12);
            if(BIT12 == 0) execute_MOVL(instruction);
            else execute_MOVLZ(instruction);
            break;
        case MOVLS_MOVH:
            BIT12 = EXTRACT_BIT(instruction, 12);
            if(BIT12 == 0) execute_MOVLS(instruction);
            else execute_MOVLH(instruction);
            break;
        case LDR:
            execute_LDR(instruction);
            break;
        case STR:
            execute_STR(instruction);
            break;
        }
}

