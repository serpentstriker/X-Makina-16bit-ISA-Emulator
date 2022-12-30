/*The Loader decodes the s-records
 * loads data/instructions in the
 * memory using the bus function
 * this module also includes the
 * memory dump function which
 * prints out all contents of
 * the memory*/

#define inst_len 2
#define address_len 4

#include "emulator.h"
FILE *infile;

int loader(int argc, char* argv[]){

    char srecord[MAX_SRec_Len];//array to store the s-record
    unsigned int ahi, ali, length, address, byte, check_sum_digit;//declare unsigned variables
    //to store the checksum
    unsigned char checksum;

    if (argc == 1){
        printf("Missing file name\n");
        (void)getchar();
        return -1;
    }
    if(fopen_s(&infile, argv[1], "r") != 0){
        printf("Error opening file  %s  -  missing file\n", argv[1]);
        (void)getchar();
        return -1;
    }

    while (fgets(srecord, MAX_SRec_Len, infile) > 0) {

        sscanf(&srecord[2], "%2x", &length);
        //address high and address low
        sscanf(&srecord[4], "%2x%2x", &ahi, &ali);
        // bit manipulation to compensate for little and big endian
        address = ahi << 8 | ali;
        // adding the address and length to the checksum variable
        checksum = ahi + ali + length;
        //for the invalid record starting with anything but an S
        if (srecord[0] != 'S') printf("Invalid Record\n");

        //verifying the count
        if (length != (strlen(srecord) - address_len) / inst_len) {
            printf("invalid count\n");
            continue;
        }
        // positioning the record right after address to decode the data
        int position = 8;
        // decrement the length by 2 because of the checksum at the end, gives us the length of the data
        length = length - 2;
        //header record
        if (srecord[1] == '0') {
            printf("Source Module: ");
            // terminate while when we reach the end of the data
            while (length != 1) {
                //finding the name if the source
                sscanf(&srecord[position], "%2x", &byte);
                //printing out each character of the source individually
                printf("%c", byte);
                //decreament length
                length--;
                // move to the next byte
                position = position + 2;
                checksum = checksum + byte;
            }
            // checksum digit here is the last 2 words in the srecord
            sscanf(&srecord[position], "%2x", &check_sum_digit);
            checksum = checksum + check_sum_digit;
            //indicate that the check sum is wrong
            if (checksum != 0xff) printf("Check sum is wrong!\n");
            // data and instructions
        } else if (srecord[1] == '1') {
            // terminate while when we reach the end of the data
            while (length != 1) {

                sscanf(&srecord[position], "%2x", &byte);
                mem.byte[address] = byte;
                address++;
                length--;
                //move to next byte
                position = position + 2;
                checksum = checksum + byte;
            }
            // checksum  digit here is the last 2 words in the srecord
            sscanf(&srecord[position], "%2x", &check_sum_digit);

            checksum = checksum + check_sum_digit;
            // check check sum and if it is wring display it
            if (checksum != 0xff) printf("Check sum is wrong!\n");
            // end record and the starting address
        } else if (srecord[1] == '9') {
            // terminate while when we reach the end of the data
            while (length != 1) {

                sscanf(&srecord[position], "%2x", &byte);
                length--;
                // move to next byte
                position = position + 2;
                checksum = checksum + byte;
            }

            PC.word = address;
            printf("\nStarting Address =  %x\n", address);
            // checksum digit here is the last 2 words in the s-record
            sscanf(&srecord[position], "%2x", &check_sum_digit);
            checksum = checksum + check_sum_digit;
            // check check sum and if it is wring display it
            if (checksum != 0xff) printf("Check sum is wrong! in the Loader\n");
            //print out the check sum
        } else printf("ERROR! in the Loader");
    }
    fclose(infile);
    printf("\n");
}

void memory_dump(){

    unsigned int lowerbound, upperbound;

    printf("Enter the lower and upper bound:\n");
    scanf("%x %x", &lowerbound, &upperbound);

    printf("******************************MEMORY DUMP******************************\n");
    for(int j = lowerbound; j <= upperbound; j+=16) {
        printf("%4.4X",j);

        for (int i = 0; i <= 16; i++) {

            printf(" %2.2X ",mem.byte[i+j]);

        }

        printf("\n");
    }
    printf("******************************MEMORY DUMP******************************\n");
}
