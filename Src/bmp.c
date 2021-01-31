#include "bmp.h"

/* Static functions used internally */

/* !! WARNING !! */
/* The following functions (readBuff, jumpTo, putpixel) are platform-dependant
    and need to be implemented by yourself, according to your system requirements ! 
    Some examples are provided.
*/

/* Reads some bytes from the file / buffer, THEN move the "cursor" forward
    (hint : this is the default ANSI-C behavior) */
static size_t readBuff(uint8_t *v, size_t s);
/* Moves the "cursor" at a certain location */
static void jumpTo(size_t pos);
/* Draws a pixel on the screen / printer / matrix / whatever
    The color is either BLACK (FALSE) or WHITE (TRUE) */
static void putpixel(uint32_t x, uint32_t y, bool color);

/* Reads the next byte on disk / from buffer */
static void readNext(uint8_t *v);
/* Reads the next TWO bytes on disk / from buffer */
static uint16_t read16(void);
/* Reads the next FOUR bytes on disk / from buffer */
static uint32_t read32(void);
/* Builds the palette from the data structure */
static void buildPalette(BMP_t *bmp);
/* Utility function used to read a single bit from a byte */
static uint8_t getBit(uint8_t i, uint8_t N);

/* ******************************** */

/* PLACE YOUR PLATFORM-DEPENDENT CODE HERE */

// Example : FATFS & SSD1306 on STM32

    /*static FIL *fp = NULL;
    void BMP_setFile(FIL *_fp) {
        fp = _fp;
    }


    size_t readBuff(uint8_t *v, size_t s) {
        size_t br;

        f_read (fp, (char*)v, s, &br);

        return br;
    }

    void readNext(uint8_t *v) {
        readBuff(v, 1);
    }

    void jumpTo(size_t pos) {
        f_lseek(fp, pos);
    }

    void putpixel(uint32_t x, uint32_t y, bool color) {
        ssd1306_DrawPixel(x, y, color);
    }*/

/* *************************************** */

// Init the struct at 0
void BMP_zeroBMP(BMP_t *bmp) {
    // Fill the structure with 0's
    memset(bmp, 0x00, sizeof(BMP_t));
}

// Read the file, and get the individual fields
BMP_Err_t BMP_parseFile(BMP_t *bmp) {
    BMP_Err_t err = 0;

    // Read all fields from the beggining of the file
    // All fields are either 16 or 32 bits.
    bmp->ID                  = read16();
    bmp->fileSize            = read32();
    bmp->appID               = read32();
    bmp->dataOffset          = read32();
    bmp->DIBHeaderSize       = read32();
    bmp->w                   = read32();
    bmp->h                   = read32();
    bmp->planeNumber         = read16();
    bmp->bitsPerPixel        = read16();
    bmp->compression         = read32();
    bmp->rawBMPDataSize      = read32();
    bmp->PPMH                = read32();
    bmp->PPMV                = read32();
    bmp->colorsInPalette     = read32();
    bmp->importantColors     = read32();

    // Sometimes the field indicating the raw BITMAP data is not generated by the CAD software.
    // (Observed with Paint.NET)
    // In this case the fiels equals 0
    // So we need to calculate it :
    if(bmp->rawBMPDataSize == 0) {
        // Compute the actual size
        //  knowing the total size of the file and the position of the data into the file.
        bmp->rawBMPDataSize = bmp->fileSize - bmp->dataOffset;
    }

    // Check if the file is valid
    err = BMP_check(bmp);

    // Exit now if an error is detected
    if(err) {
        return err;
    }

    // Build the color palette
    buildPalette(bmp);

    // Allocate the buffer containing the actual picture data
    bmp->data = (uint8_t*)malloc(bmp->rawBMPDataSize);

    return err;
}

// Build the color palette from the fields
void buildPalette(BMP_t *bmp) {
    // The default palette is :
    //  - 0 : Black
    //  - 1 : White
    bmp->BWPalette[0] = 0;
    bmp->BWPalette[1] = 1;

    // If not palette is specified, stay with the default one
    // (In this case there is 0 colors in the palette)
    if(bmp->colorsInPalette == 0) {
        return;
    }

    // The actual palette interpretation is a bit wanky.
    // I assume that :
    //  - Black = 0x000000
    //  - White = 0xFFFFFF
    // The alpha channel is not taken into account
    // Go to the first entry of the palette (Color '0')
    jumpTo(0x36);
    bmp->BWPalette[0] = (read32() & 0x00FFFFFF)?1:0;
    // Go to the second entry of the palette (Color '1')
    jumpTo(0x3A);
    bmp->BWPalette[1] = (read32() & 0x00FFFFFF)?1:0;
}

// Check for file inconsistencies or invalid fields 
BMP_Err_t BMP_check(BMP_t *bmp) {
    int res = 0;

    // Wrong signature ! Are you sure it is a .BMP file ?
    if(bmp->ID != 0x4D42) {
        res = BMP_Err_Bad_Signature;
    }

    // WILL happen if the BITMAP file is not B&W 1-bit color :/
    if(bmp->bitsPerPixel > 1 || bmp->compression != 0) {
        res = BMP_Err_Not_BW;
    }

    // Some junky palette ? Yikes ! (Once again in case of color BITMAP)
    if(bmp->colorsInPalette != 0 && bmp->colorsInPalette != 2) {
        res = BMP_Err_Invalid_Palette;
    }

    return res;
}

// Read the actual picure data from the file
BMP_Err_t BMP_readData(BMP_t *bmp) {
    BMP_Err_t err = BMP_Err_OK;

    // Go to the data location in the file
    jumpTo(bmp->dataOffset);

    // Copy the data to the buffer
    if(readBuff(bmp->data, bmp->rawBMPDataSize) != bmp->rawBMPDataSize) {
        // An error occurs if the count is not the same
        err = BMP_Err_Data_Read;
    }

    return err;
}

// Frees the buffer
void BMP_release(BMP_t *bmp) {
    free(bmp->data);
}

// Here comes the heavy lifting :]
// Draws the bitmap stored in the buffer to the screen
BMP_Err_t BMP_blit(BMP_t *bmp, uint32_t _x, uint32_t _y) {
    // Exit immediately if the buffer is not allocated
    if(!bmp->data) {
        return BMP_Err_Empty_Data;
    }

    // Compute the width of the lines (in bytes)
    uint32_t lineWidth = bmp->w / 8;
    if(bmp->w % 8) {
        lineWidth += 1;
    }

    // Compute the width of the lines (in bytes) IN THE BITMAP FILE,
    //  so with a padding of FOUR BYTES
    uint32_t BMPLineWidth;
    BMPLineWidth = ((lineWidth / 4) * 4);
    if(lineWidth % 4) {
        BMPLineWidth += 4;
    }

    // Going through rows (y) / columns (x)
    //  in the common way of thinking (from top-left to bottom-right)
    for(uint32_t y = 0; y < bmp->h; y ++) {
        // Don't forget : BITMAP files are stored backward (first line last)
        // So we need to (invert the line order)
        uint32_t BMPLine = bmp->h - (y + 1);
        // Compute the address of the first byte of the line
        uint32_t lineByteOffset = BMPLine * BMPLineWidth;

        for(uint32_t x = 0; x < bmp->w; x ++) {
            // Get the byte index in the current line
            uint32_t offsetInLine = x / 8;
            // Compute the adress of the current byte in the BITMAP file
            uint32_t i = lineByteOffset + offsetInLine;
            // Finally, get the BIT index (0-8) in the previously calculated byte
            uint8_t bitIndex = 7 - (x % 8);

            // Retrieve the byte
            uint8_t byte = bmp->data[i];

            // Drawing pixel :
            //  - Computing his actual coordinates
            //  - Get the color from the pallete according to the bit value
            //      - Extract the single bit from the byte
            putpixel(_x + x, _y + y, bmp->BWPalette[getBit(bitIndex, byte)]);
        }
    }

    return 0;
}

// Get the Nth bit from a byte
uint8_t getBit(uint8_t i, uint8_t N) {
    return (N >> i) & 0x01;
}

// Reads TWO bytes from the file
uint16_t read16(void) {
    uint16_t v = 0;

    for(uint8_t i = 0; i < 2; i ++) {
        // Tricky pointer manipulation :
        //  Convert a 16-bit variable to an array of 8-bit variables
        //  This allows a more direct data transfer,
        //      only needing to increment the index of the array
        readNext((uint8_t*)(&v) + i);
    }

    return v;
}

// Reads FOUR bytes from the file
uint32_t read32(void) {
    uint32_t v = 0;

    for(uint8_t i = 0; i < 4; i ++) {
        readNext((uint8_t*)(&v) + i);
    }

    return v;
}
