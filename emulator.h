//
// Created by 19023 on 2022-06-03.
//

#ifndef EMULATOR_EMULATOR_H
#define EMULATOR_EMULATOR_H

/*
 * Rehan Khalid
 * Assignment 1 X-Makina Emulator
 * ECED 3403 Computer Architecture
 */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

// maximum length of an s-record
#define MAX_SRec_Len 100
#define REGCOUNT 2
#define REG_CONST 16
#define REG 0

#define PC regfile[0][15]
#define SP regfile[0][14]
#define BP regfile[0][12]
#define LR regfile[0][13]


//extracting opcode for opcode
#define OPCODE_3MSB(x)  (((x) >> 13) & 0X07)
//macro for extracting any bit, takes 2 arguments
#define EXTRACT_BIT(INSTRUCTION, POSITION) (((INSTRUCTION >> POSITION)) & 1)
//extracts bits 3 to 5
#define BIT3TO5(x) (((x)>> 3) & 7)
//extracts the conditional code for cex(CCCC)
#define CEX_CODE(x) (((x) >> 6) & 0xF)
//extracts bits 0 to 2
#define BIT0TO2(x)  ((x)  & 7)
//extracts bits to determine the mov instructions
#define MOVX_DATA(x) (((x) >> 3) & 0xFF)
//extracts bits 8 to 11
#define BIT8TO11(x) (((x) >> 8) & 0xF)
//extracts bits 10 to 12
#define BIT10TO12(x) (((x) >> 10) & 7)
//extracts the offset for BL instruction
#define BLOFFSET(x) ((x) & 0x1fff)
//extracts the offset for BR instruction
#define BROFFSET(x) ((x) & 0x3ff)
//extracts the offset for LDR and STR instruction
#define LDROFFSET(x) ((x) >> 7 & 0x1f)
//extracts the most significant bit of a word
#define MSBofword(x) ((x >> 15) & 1)
//extracts the most significant bit of a byte
#define MSBofbyte(x) ((x >> 7) & 1)
//extracts bits 8 to 10
#define BIT8TO10(x) ((x>>8) & 7)
//extracts bits 9 to 11
#define BIT9TO11(x) ((x>>9) & 7)
#define BIT3TO4(x)  ((x>>3) & 3)
#define BIT0TO3(x)  ((x) & 15)
#define BIT5TO7(x)  ((x>>5) & 7)
#define BIT13TO15(x)  ((x>>13) & 7)
//for the decoder to decode instructions
enum INSTRUCTION{
    BL,
    BR_TO_SWAP,
    LD_ST,
    addressing_instructions,
    MOVL_MOVLZ,
    MOVLS_MOVH,
    LDR,
    STR
};

enum ACTION{read, write};
enum SIZE{word, byte};

//MEMORY UNION for main memory
union MEMORY{
    unsigned char byte[0x10000];
    unsigned short word[0x8000];
};
extern union MEMORY mem;

//word or byte to access the register file as word or a byte
union word_byte
{
    unsigned short word;
    unsigned char byte[2];
};

extern union MEMORY mem;

//PSW struct to store PSW bits
typedef struct PSW{
    unsigned short V:1; //overflow
    unsigned short N:1; //negative
    unsigned short C:1; //carry
    unsigned short Z:1; //zero
    unsigned short SLP; //sleep
    unsigned short FLT; //fault
    unsigned short P_PRI; //previous priority
    unsigned short C_PRI; //current priority
} psw;

//CEX structure for the CEX operation
enum state{FALSE = 0, TRUE = 1, OFF = 2};
typedef struct CEX{
    enum state STATE;
    unsigned short TTT;
    unsigned short FFF;
} cex;

extern cex CEX;

//global register file
extern union word_byte regfile[2][16];

//global Memory buffer register, Memory address register
extern unsigned short mbr, mar;

extern int CLOCK;

/*The bus function is supposed to take Memory address register, and a pointer to the Memory buffer register
 * the pointer avoids us returning anything, then the indication weather it is a read or a write operation
 * and weather the operation is being performed on a byte or a word, bus function is used whenever the CPU
 * or other modules interact with memory
 * */
void bus(unsigned short mar, unsigned short* mbr, enum ACTION rw, enum SIZE bw);

/*update_psw function changes the bits of the program status word
 * based on the result of the arithmetic operations performed
 * */
void update_psw(unsigned short src, unsigned short dst, unsigned short res, unsigned short wb);

//Loader decodes s-records and loads the instructions in to the memory
int loader();

//The memory dump function prints out the memory which has the data and instructions
void memory_dump();

//fetch fetches the instruction from the memory using the PC(program counter) and stores them in the register MBR
void fetch();

//decoder decodes the instructions based on the MSBs of the instruction and returns an enum value
enum INSTRUCTION decoder();

//takes enum as an argument and further decodes the instructions and performs executions
void execute(enum INSTRUCTION inst);

//this function prints out the register file
void printregisters();

//this psw function is used to do logical operation
void update_psw_logic(unsigned short res, unsigned short wb);

//displays PSW
void display_psw();

#endif //EMULATOR_EMULATOR_H
