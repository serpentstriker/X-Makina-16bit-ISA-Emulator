/*This function takes the instruction in MBR and decodes it*/
#include "emulator.h"
enum INSTRUCTION decoder(){
    CLOCK++;
    unsigned short instruction;
    unsigned int opcode;
    //instruction acts like a buffer register to store the instruction
    instruction = mbr;
    opcode = OPCODE_3MSB(instruction);

    // here we will switch on the opcode to which will have 7 cases however case 3 does not exist
    switch(opcode){
        case BL:
            // instruction is BL return
            return BL;
        case BR_TO_SWAP:
            // instructions are from BR to SWAP
            // further decoding will be required at execute
            return BR_TO_SWAP;
        case addressing_instructions:
            return addressing_instructions;
        case LD_ST:
            // instruction is either LD or ST
            // further decoding will be required at execute
            return LD_ST;
        case MOVL_MOVLZ:
            // instruction is either MOVL OR MOVLZ
            // further decoding will be required at execute
            return MOVL_MOVLZ;
        case MOVLS_MOVH:
            // instruction is either MOVLS or MOVH
            // further decoding will be required at execute
            return MOVLS_MOVH;
        case LDR:
            // instruction is LDR
            return LDR;
        case STR:
            // instruction is STR
            return STR;
    }
}