/*
    This sample code is supposed to draw a picture stored in the "picture.bmp" file on the screen.
    This example shows how to use the dirtybitmap library with the FATFS and SSD1306 libraries on STM32.
*/

void drawBitmap(void) {
    // Clear the screen
    ssd1306_Fill(Black);
    ssd1306_SetCursor(0, 0);
    
    FIL fil;    // File pointer
    BMP_t bmp;  // The variable storing the bitmap file

    // Init the structure with 0's
    // This is useful later to test if all fields are valid
    BMP_zeroBMP(&bmp);

    // Opening file
    if(f_open(&fil, "picture.bmp", FA_READ) != FR_OK) {
        // If unable to open file, display an error message
        ssd1306_WriteString("Failed to open file !", Font_6x8, White);
        ssd1306_UpdateScreen();

        return;
    }

    // Pass the file to the library
    // !! This is your job to implement such a function as it is not provided by the library !!
    BMP_setFile(&fil);

    // Parse the file, and store the return code.
    // BITMAP data will be copied from the file to the BMP_t struct
    BMP_Err_t err;
    err = BMP_parseFile(&bmp);

    if(err) {
        // If an error occured, display an error message
        ssd1306_WriteString("INVALID FILE !", Font_6x8, White);
        ssd1306_UpdateScreen();

        return;
    }
    
    // Read the actual picture data
    err = BMP_readData(&bmp);

    // You can close the file now as all data are copied
    f_close(&fil);

    if(err) {
        // If an error occured, display an error message
        ssd1306_WriteString("UNABLE TO READ DATA !", Font_6x8, White);
        ssd1306_UpdateScreen();

        return;
    }

    // Draw the bitmap from the point (10, 5)
    BMP_blit(&bmp, 10, 5);

    // When the picture is no more needed, don't forget to free the buffer !
    BMP_release(&bmp);

    // Update the screen !
    ssd1306_UpdateScreen();
}