#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"

int main()
{
    BMP_t bmp;

    BMP_zeroBMP(&bmp);

    printf("Opening file\n");

    FILE *fp;
    fp = fopen("bitmap.bmp", "rb");

    if(!fp) {
        printf("Unable to open file\n");

        return -1;
    }

    BMP_setFile(fp);

    BMP_Err_t err;

    printf("Parsing file\n");

    err = BMP_parseFile(&bmp);

    if(err) {
        printf("Invalid file ! Err (%u)\n", err);

        return -2
    }

    printf("Reading data\n");

    err = BMP_readData(&bmp);

    if(err) {
        printf("Unable to read data ! Err (%u)\n", err);

        return -3;
    }

    printf("Closing file\n");

    fclose(fp);

    printf("Drawing picture\n");

    BMP_blit(&bmp, 0, 0);

    printf("\n\nReleasing buffer\n");

    BMP_release(&bmp);

    printf("\n\n");

    return 0;
}
