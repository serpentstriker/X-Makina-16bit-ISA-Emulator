/*Fetch uses the bus function to read the instruction and put it in the PC*/
#include "emulator.h"

unsigned short mbr, mar; // declaring the extern register shorts that were already declared in the header file
int CLOCK;
void fetch(){
    CLOCK++;
    mar = PC.word; //assigning memory address register the value of
    bus(mar,&mbr,read,word);
    PC.word = PC.word + 2;
}
