/*This module prints out the registers*/
#include "emulator.h"

void printregisters() {

    for (int i = 0; i <= 7; i++) {
        printf("R%d:    %4.4X     |   A%d:   %4.4X     |    C%d:    %4.4X\n", i, regfile[0][i].word, i, regfile[0][i+8].word, i,
               regfile[1][i].word);
    }
    printf("MAR =   %4X     |       MBR =   %4X\n", mar, mbr);
}