/*The module ahs the bus function and the update psw function*/

#include "emulator.h"
psw PSW;

/*Reads or writes to the memory*/
void bus(unsigned short mar, unsigned short* mbr, enum ACTION rw, enum SIZE bw){
    CLOCK+=3;
    //increment clock
    //it is a word
    if(bw == word){
        //if we are reading a word
        if(rw == read){
            *mbr = mem.word[mar>>1];
        }
        //if we are writing a word
        else{
            mem.word[mar>>1] = *mbr;
        }
    }
    //it is a byte
    else{
        //reading a byte
        if(rw == read){
            *mbr = mem.byte[mar]&0xFF;
        }
        //writing a byte
        else{
            mem.byte[mar] = (*mbr)&0xFF;
        }
    }
}

/*Updates the program status word for arithmetic instructions */
void update_psw(unsigned short src, unsigned short dst, unsigned short res, unsigned short wb){
    unsigned short src_bit, dst_bit, res_bit;
    //this is like a map to set carry and overflow bits
    unsigned carry[2][2][2] = {0,0,1,0,1,0,1,1};
    unsigned overflow[2][2][2] = {0,1,0,0,0,0,1,0};
    //this if statement is when a word is encountered
    if (wb == 0) {
        src_bit = MSBofword(src);
        dst_bit = MSBofword(dst);
        res_bit = MSBofword(res);
        //getting the 15th bit of the result which depicts the signed bit
        //if the result is 0 then the psw bit z will be set to 0
        PSW.N = (res_bit == 1);
        PSW.Z = (res == 0);
    }
    else { //this else statement is for when a byte is encountered
        src_bit = MSBofbyte(src);
        dst_bit = MSBofbyte(dst);
        res_bit = MSBofbyte(res);
        //getting the 7th position bit of the result which depicts the signed bit
        PSW.N = (res_bit == 1);
        //getting the 8 bits of the result if they are zero then PSW.Z will be 1
        PSW.Z = (res == 0);
    }
    //this just calculates the carry and assigns the bit to it
    PSW.C = carry[src_bit][dst_bit][res_bit];
    //this finds overflow bit
    PSW.V = overflow[src_bit][dst_bit][res_bit];
}

/*Updates the program status word for logical operations instructions */
void update_psw_logic(unsigned short res, unsigned short wb){
    unsigned short res_bit;

    //this if statement is when a word is encountered
    if (wb == 0) {
        //getting the 15th bit of the result which depicts the signed bit
        res_bit = MSBofword(res);
        PSW.N = (res_bit == 1);
        //if the result is 0 then the psw bit z will be set to 0
        PSW.Z = (res == 0);
    }
    else { //this else statement is for when a byte is encountered
        //getting the 7th position bit of the result which depicts the signed bit
        res_bit = MSBofbyte(res);
        PSW.N = (res_bit == 1);
        //if the result is 0 then the psw bit z will be set to 0
        PSW.Z = (res == 0);
    }

}

/*This function displays PSW bits*/
void display_psw(){
    printf("PSW   V: %u    N: %u    Z: %u    C: %u\n", PSW.V, PSW.N, PSW.Z, PSW.C);
}