#ifndef BMP_H
#define BMP_H

/* *** Place your custom includes here *** */
// Eg. FATFS & SSD1306 libs
// #include "ssd1306.h"
// #include "fatfs.h"
/* *************************************** */

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* The BITMAP strucure */
/*  
    Not all fields are used
    but they are all filled from the file.
*/
typedef struct {
    uint16_t ID;
    uint32_t fileSize;
    uint32_t appID;
    uint32_t dataOffset;
    uint32_t DIBHeaderSize;
    uint32_t w;
    uint32_t h;
    uint16_t planeNumber;
    uint16_t bitsPerPixel;
    uint32_t compression;
    uint32_t rawBMPDataSize;
    uint32_t PPMH;
    uint32_t PPMV;
    uint32_t colorsInPalette;
    uint32_t importantColors;

    bool BWPalette[2];

    uint8_t *data;
} BMP_t;

/* A little list of possible errors */
typedef enum {
    BMP_Err_OK              = 0,    // No errors.
    BMP_Err_Bad_Signature   = -1,   // The file doesn't start with "BM".
    BMP_Err_Not_BW          = -2,   // The format is not 1 bit / pixel.
    BMP_Err_Invalid_Palette = -3,   // The palette is not valid (only 0 & 2 colors allowed).
    BMP_Err_Empty_Data      = -4,   // The buffer is empty. Something went wrong, or the file was not loaded.
    BMP_Err_Data_Read       = -5    // Unable to read data.
} BMP_Err_t;

/* *** Place your custom disk / buffer access functions here *** */
// Eg. FATFS file access
// void BMP_setFile(FIL *_fp);
/* ************************************************************* */

/*
*** PLEASE NOTE *** 
    Some plateform-dependant code is needed for :
        - Disk / Buffer acccess
        - Screen drawing (drawing a single pixel)
    See the beggining of the "bmp.c" file.
*******************
*/

/* This function fills the BMP_t struct with 
    0's, just in case. */
/*
    Param : Pointer to the BMP_t object
    Ret   : None
*/
void BMP_zeroBMP(BMP_t *bmp);

/* This function parses the BITMAP file 
    (or the buffer containing the file)
    and fills the BMP_t struct. */
/*
    Param : Pointer to the BMP_t object
    Ret   : Error code
*/
BMP_Err_t BMP_parseFile(BMP_t *bmp);

/* This function checks the BMP_t object 
    by looking for invalid fields */
/*
    Param : Pointer to the BMP_t object
    Ret   : Error code
*/
BMP_Err_t BMP_check(BMP_t *bmp);

/* This function loads the proper
    pixel data from the file 
    NOTE : After calling this function,
        the file is no more needed, so it can be closed.*/
/*
    Param : Pointer to the BMP_t object
    Ret   : Error code
*/
BMP_Err_t BMP_readData(BMP_t *bmp);

/* This function displays the BITMAP to the screen 
    Note : Some platform-dependent code is needed ! */
/*
    Params : Pointer to the BMP_t object, Coordinates from where to draw the bitmap (_x, _y)
    Ret   : Error code
*/
BMP_Err_t BMP_blit(BMP_t *bmp, uint32_t _x, uint32_t _y);

/* This function frees the pixel buffer */
/*
    Param : Pointer to the BMP_t object
    Ret   : None
*/
void BMP_release(BMP_t *bmp);

#endif // BMP_H