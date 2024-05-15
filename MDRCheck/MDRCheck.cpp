#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned int uint;

void error(int errorcode);

int main(int argc, char* argv[]) {
    uint i, j;
    uint chksum;
    uint blankSectors = 0, badCSR = 0, badCSH = 0, badCSRH = 0;
    FILE* fp_in;
    unsigned char* mdr;
    //
    if (argc < 2) error(1);
    //
    if ((fp_in = fopen(argv[1], "rb")) == NULL) error(2); // cannot open mdr for read
    fseek(fp_in, 0, SEEK_END); // jump to the end of the file to get the length
    int filesize = ftell(fp_in); // get the file size
    rewind(fp_in);
    //if(filesize!=137923) error(3);
    //
    if ((mdr = (unsigned char*)malloc(filesize * sizeof(unsigned char))) == NULL) error(4);
    if (fread(mdr, sizeof(unsigned char), filesize, fp_in) != filesize) error(5);
    fclose(fp_in);
    //

    //
    printf("Cartridge Name: \"");
    j = 4;
    while (j < 137926) {
        if (mdr[j] != 0x00) break;
        j += 543;
    }
    if (j > 137923) {
        printf("Error: MDR does not contain any valid sectors\n");
        return 0;
    }
    // rewind
    // create blank mdr with name
    for (i = 0; i < 10; i++) printf("%c", mdr[j + i]);
    printf("\"\n");
    //checksum test
    i = 0;
    do {
        //for(i=0;i<254;i++) {
        if (mdr[1 + i * 543] != 0) {
            for (j = 0, chksum = 0; j < 14; j++) {
                chksum += mdr[j + i * 543];
                chksum = chksum % 255;
            }
            printf("Sector %3d -> ", mdr[1 + i * 543]);
            printf("HC:%02x,%02x(%c) | ", chksum, mdr[14 + i * 543], (chksum - mdr[14 + i * 543] == 0) ? 'o' : 'x');
            if (chksum - mdr[14 + i * 543]) badCSH++;
            printf("RF:%02x | ", mdr[15 + i * 543]);
            if (mdr[18 + i * 543] * 256 + mdr[17 + i * 543] > 512) {
                printf("RL:### | ");
            }
            else {
                printf("RL:%3d | ", mdr[18 + i * 543] * 256 + mdr[17 + i * 543]);
            }
            for (j = 15, chksum = 0; j < 29; j++) {
                chksum += mdr[j + i * 543];
                chksum = chksum % 255;
            }
            printf("RC:%02x,%02x(%c) | ", chksum, mdr[29 + i * 543], (chksum - mdr[29 + i * 543] == 0) ? 'o' : 'x');
            if (chksum - mdr[29 + i * 543]) badCSRH++;
            for (j = 30, chksum = 0; j < 542; j++) {
                //printf("%02x ",mdr[j+i*543]);
                chksum += mdr[j + i * 543];
                chksum = chksum % 255;
                //printf("%02x ",chksum);
            }
            printf("DC:%02x,%02x(%c)\n", chksum, mdr[542 + i * 543], (chksum - mdr[542 + i * 543] == 0) ? 'o' : 'x');
            if (chksum - mdr[542 + i * 543]) badCSR++;

        }
        else {
            blankSectors++;
        }
        i++;
    } while (i * 543 < filesize - 543);
    printf("Total Sectors Read:%3d\nBlank Sectors:%3d\nBad Checksums: Sector:%3d, Header:%3d, Data:%3d\n", i, blankSectors, badCSH, badCSRH, badCSR);
    //if(blankSectors) printf("Image contains %d blank sectors\n",blankSectors);
    // if(badCSH) printf("Image contains %d sector headers with bad checksums\n",badCSH);
    // if(badCSRH) printf("Image contains %d sector record headers with bad checksums\n",badCSRH);
    // if(badCSR) printf("Image contains %d sector records with bad checksums\n",badCSR);
    free(mdr);
    return 0;
}

void error(int errorcode) {
    fprintf(stdout, "[E%02d]\n", errorcode);
    exit(errorcode);
}