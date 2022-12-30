#include <stdio.h>
#include "emulator.h"

//2-D array to store Registers and constants
union word_byte regfile[REGCOUNT][REG_CONST] = {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                                                {0,1,2,4,8,16,32,-1,0,0,0,0,0,0,0,0}};

//memory initialized to zero
union MEMORY mem = {0};

int main() {
    CLOCK = 0;
    CEX.STATE = OFF;
    unsigned short breakpoint;
    int input;
    breakpoint = 0XFFFF;
    loader();
    enum INSTRUCTION decoded_instruction;
    printf("\n\n    Welcome to the X-Makina Emulator\n");
    printf("    Written by CE. Rehan Khalid B00826172\n\n\n");
    while(1) {
        printf("    Enter 1 to display Registers\n");
        printf("    Enter 2 to display Memory Contents\n");
        printf("    Enter 3 to display Program Status Word\n");
        printf("    Enter 4 to set a break point\n");
        printf("    Enter 5 to run the Emulator\n");
        printf("    Enter 6 to set the Program Counter manually\n");
        printf("    Enter 7 to Exit\n\n");
        scanf("%d", &input);


        switch (input) {
            case 1:
                printregisters();
                break;
            case 2:
                memory_dump();
                break;
            case 3:
                display_psw();
                break;
            case 4:
                printf("Breakpoint =\n");
                scanf("%4x", &breakpoint);
                break;
            case 5:
                do{
                    if (CEX.STATE == OFF) {
                        fetch();
                        decoded_instruction = decoder();
                        execute(decoded_instruction);
                    }
                        //if the CEX state is on and is equal to true
                    else if (CEX.STATE == TRUE){
                        while (CEX.TTT > 0 && CEX.STATE != OFF){
                            fetch();
                            decoded_instruction = decoder();
                            execute(decoded_instruction);
                            CEX.TTT--;
                        }
                        while (CEX.FFF) {
                            fetch();
                            CEX.FFF--;
                        }
                        CEX.STATE = OFF;
                    }
                        //if the CEX state is on and is equal to false
                    else if (CEX.STATE == FALSE){
                        while (CEX.TTT > 0 && CEX.STATE != OFF){
                            fetch();
                            CEX.TTT--;
                        }
                        while (CEX.FFF > 0) {
                            fetch();
                            decoded_instruction = decoder();
                            execute(decoded_instruction);
                            CEX.FFF--;
                        }
                        CEX.STATE = OFF;
                    }
                }while(PC.word != breakpoint);
                printf("Final CLOCK = %d\n\n", CLOCK);
                break;
            case 6:
                scanf("%x", &PC);
                break;
            case 7:
                return 0;
        }

    }
}
