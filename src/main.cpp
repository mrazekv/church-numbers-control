#include <Arduino.h>

// Graphics and font library
#include <TFT_eSPI.h>
#include "FreeFont.h"
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();  // Invoke library
void header(const char *string, uint16_t color);

// Draw a + mark centred on x,y
void drawDatumMarker(int x, int y);

// put function declarations here:
int myFunction(int, int);

void setup() {
  // put your setup code here, to run once:
  int result = 5;
  Serial.begin(9600);
  Serial.print("The result of myFunction(2, 3) is: ");
  Serial.println(result);

   // Initialise the TFT after the SD card!
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(0, 0);
// set font smooth 
tft.setFreeFont(FF32);
  tft.println("Hello, World!");
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.setTextSize(3);
  tft.setCursor(0, 50);
  tft.println("Welcome to TFT_eSPI!");
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextSize(4);
  tft.setCursor(0, 100);
  tft.println("Enjoy your day!");
  tft.setTextColor(TFT_BLUE, TFT_BLACK);  
}
void loop() {

  Serial.println("loop1");
  int xpos =  0;
  int ypos = 40;
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  tft.setTextSize(1);

  tft.setFreeFont(FSSB24);       // Select Free Serif 24 point font
  tft.setTextDatum(TC_DATUM); // Centre text on x,y position

  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString("123", 50, 30, GFXFF);  // Draw the text string in the selected GFX free font

  
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.setFreeFont(FSSB18); 
  tft.drawString("12", 125, 30+15, GFXFF);  // Draw the text string in the selected GFX free font

  // draw rounded rectangle
  tft.drawRoundRect(10, 80, 60, 20, 5, TFT_RED);
  tft.setTextColor(TFT_WHITE, TFT_RED);
  tft.setTextDatum(MC_DATUM); // Centre text on x,y position
  tft.setFreeFont(FS9);       // Select Free Serif 24 point font
  tft.drawString("1:12345", 10+30, 95, GFXFF);  // Draw the text string in the selected GFX free font

  /*tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setCursor(xpos, ypos + 40);
  tft.setTextFont(8); 
  tft.println("123");*/
  delay(4000);
  return;
  // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
  // Select different fonts to draw on screen using the print class
  // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

  header("Using print() method", TFT_NAVY);

  // For comaptibility with Adafruit_GFX library the text background is not plotted when using the print class
  // even if we specify it.
  tft.setTextColor(TFT_YELLOW);
  tft.setCursor(xpos, ypos);    // Set cursor near top left corner of screen

  tft.setFreeFont(TT1);     // Select the orginal small TomThumb font
  tft.println();             // Move cursor down a line
  tft.print("The really tiny TomThumb font");    // Print the font name onto the TFT screen
  tft.println();
  tft.println();

  tft.setFreeFont(FSB9);   // Select Free Serif 9 point font, could use:
  // tft.setFreeFont(&FreeSerif9pt7b);
  tft.println();          // Free fonts plot with the baseline (imaginary line the letter A would sit on)
  // as the datum, so we must move the cursor down a line from the 0,0 position
  tft.print("Serif Bold 9pt");  // Print the font name onto the TFT screen

  tft.setFreeFont(FSB12);       // Select Free Serif 12 point font
  tft.println();                // Move cursor down a line
  tft.print("Serif Bold 12pt"); // Print the font name onto the TFT screen

  tft.setFreeFont(FSB18);       // Select Free Serif 12 point font
  tft.println();                // Move cursor down a line
  tft.print("Serif Bold 18pt"); // Print the font name onto the TFT screen

  tft.setFreeFont(FSB24);       // Select Free Serif 24 point font
  tft.println();                // Move cursor down a line
  tft.print("Serif Bold 24pt"); // Print the font name onto the TFT screen


  delay(4000);

  // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
  // Now use drawString() so we can set font background colours and the datum
  // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

  header("Using drawString()", TFT_BLACK);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  tft.setTextDatum(TC_DATUM); // Centre text on x,y position

  xpos = tft.width() / 2; // Half the screen width
  ypos = 50;

  tft.setFreeFont(FSB9);                              // Select the font
  tft.drawString("Serif Bold 9pt", xpos, ypos, GFXFF);  // Draw the text string in the selected GFX free font
  ypos += tft.fontHeight(GFXFF);                      // Get the font height and move ypos down

  tft.setFreeFont(FSB12);
  tft.drawString("Serif Bold 12pt", xpos, ypos, GFXFF);
  ypos += tft.fontHeight(GFXFF);

  tft.setFreeFont(FSB18);
  tft.drawString("Serif Bold 18pt", xpos, ypos, GFXFF);
  ypos += tft.fontHeight(GFXFF);

  tft.setFreeFont(FSB24);
  tft.drawString("Serif Bold 24pt", xpos, ypos, GFXFF);
  ypos += tft.fontHeight(GFXFF);

  // Set text padding to 100 pixels wide area to over-write old values on screen
  tft.setTextPadding(100);
  for (int i = 0; i <= 20; i++) {
    tft.drawFloat(i / 10.0, 1, xpos, ypos, GFXFF);
    delay (200);
  }

  delay(4000);

  // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
  // Same again but with colours that show bounding boxes
  // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


  header("With background", TFT_DARKGREY);

  tft.setTextColor(TFT_YELLOW, TFT_BLACK);

  tft.setTextDatum(TC_DATUM); // Centre text on x,y position

  xpos = tft.width() / 2; // Half the screen width
  ypos = 50;

  tft.setFreeFont(FSB9);                              // Select the font
  tft.drawString("Serif Bold 9pt", xpos, ypos, GFXFF);  // Draw the text string in the selected GFX free font
  ypos += tft.fontHeight(GFXFF);                        // Get the font height and move ypos down

  tft.setFreeFont(FSB12);
  tft.drawString("Serif Bold 12pt", xpos, ypos, GFXFF);
  ypos += tft.fontHeight(GFXFF);

  tft.setFreeFont(FSB18);
  tft.drawString("Serif Bold 18pt", xpos, ypos, GFXFF);
  ypos += tft.fontHeight(GFXFF);

  tft.setFreeFont(FSBI24);
  tft.drawString("Bold Italic 24pt", xpos, ypos, GFXFF);
  ypos += tft.fontHeight(GFXFF);

  // Set text padding to 100 pixels wide area to over-write old values on screen
  tft.setTextPadding(100);
  for (int i = 0; i <= 20; i++) {
    tft.drawFloat(i / 10.0, 1, xpos, ypos, GFXFF);
    delay (200);
  }

  delay(4000);

  // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
  // Now show setting the 12 datum positions works with free fonts
  // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

  // Numbers, floats and strings can be drawn relative to a datum
  header("Text with a datum", TFT_BLACK);
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.setFreeFont(FSS12);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("[Top left]", 160, 120, GFXFF);
  drawDatumMarker(160,120);
  delay(1000);

  tft.fillRect(0, 80, 320, 80, TFT_BLACK);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("[Top centre]", 160, 120, GFXFF);
  drawDatumMarker(160,120);
  delay(1000);

  tft.fillRect(0, 80, 320, 80, TFT_BLACK);
  tft.setTextDatum(TR_DATUM);
  tft.drawString("[Top right]", 160, 120, GFXFF);
  drawDatumMarker(160,120);
  delay(1000);

  tft.fillRect(0, 80, 320, 80, TFT_BLACK);
  tft.setTextDatum(ML_DATUM);
  tft.drawString("[Middle left]", 160, 120, GFXFF);
  drawDatumMarker(160,120);
  delay(1000);

  tft.fillRect(0, 80, 320, 80, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("[Middle centre]", 160, 120, GFXFF);
  drawDatumMarker(160,120);
  delay(1000);

  tft.fillRect(0, 80, 320, 80, TFT_BLACK);
  tft.setTextDatum(MR_DATUM);
  tft.drawString("[Middle right]", 160, 120, GFXFF);
  drawDatumMarker(160,120);
  delay(1000);

  tft.fillRect(0, 80, 320, 80, TFT_BLACK);
  tft.setTextDatum(BL_DATUM);
  tft.drawString("[Bottom left]", 160, 120, GFXFF);
  drawDatumMarker(160,120);
  delay(1000);

  tft.fillRect(0, 80, 320, 80, TFT_BLACK);
  tft.setTextDatum(BC_DATUM);
  tft.drawString("[Bottom centre]", 160, 120, GFXFF);
  drawDatumMarker(160,120);
  delay(1000);

  tft.fillRect(0, 80, 320, 80, TFT_BLACK);
  tft.setTextDatum(BR_DATUM);
  tft.drawString("[Bottom right]", 160, 120, GFXFF);
  drawDatumMarker(160,120);
  delay(1000);

  tft.fillRect(0, 80, 320, 80, TFT_BLACK);
  tft.setTextDatum(L_BASELINE);
  tft.drawString("[Left baseline]", 160, 120, GFXFF);
  drawDatumMarker(160,120);
  delay(1000);

  tft.fillRect(0, 80, 320, 80, TFT_BLACK);
  tft.setTextDatum(C_BASELINE);
  tft.drawString("[Centre baseline]", 160, 120, GFXFF);
  drawDatumMarker(160,120);
  delay(1000);

  tft.fillRect(0, 80, 320, 80, TFT_BLACK);
  tft.setTextDatum(R_BASELINE);
  tft.drawString("[Right baseline]", 160, 120, GFXFF);
  drawDatumMarker(160,120);
  delay(1000);

  //while(1);
  delay(8000);

}

// Print the header for a display screen
void header(const char *string, uint16_t color)
{
  tft.fillScreen(color);
  tft.setTextSize(1);
  tft.setTextColor(TFT_MAGENTA, TFT_BLUE);
  tft.fillRect(0, 0, 320, 30, TFT_BLUE);
  tft.setTextDatum(TC_DATUM);
  tft.drawString(string, 160, 2, 4); // Font 4 for fast drawing with background
}

// Draw a + mark centred on x,y
void drawDatumMarker(int x, int y)
{
  tft.drawLine(x - 5, y, x + 5, y, TFT_GREEN);
  tft.drawLine(x, y - 5, x, y + 5, TFT_GREEN);
}


// There follows a crude way of flagging that this example sketch needs fonts which
// have not been enabled in the User_Setup.h file inside the TFT_HX8357 library.
//
// These lines produce errors during compile time if settings in User_Setup are not correct
//
// The error will be "does not name a type" but ignore this and read the text between ''
// it will indicate which font or feature needs to be enabled
//
// Either delete all the following lines if you do not want warnings, or change the lines
// to suit your sketch modifications.

#ifndef LOAD_GLCD
//ERROR_Please_enable_LOAD_GLCD_in_User_Setup
#endif

#ifndef LOAD_FONT2
//ERROR_Please_enable_LOAD_FONT2_in_User_Setup!
#endif

#ifndef LOAD_FONT4
//ERROR_Please_enable_LOAD_FONT4_in_User_Setup!
#endif

#ifndef LOAD_FONT6
//ERROR_Please_enable_LOAD_FONT6_in_User_Setup!
#endif

#ifndef LOAD_FONT7
//ERROR_Please_enable_LOAD_FONT7_in_User_Setup!
#endif

#ifndef LOAD_FONT8
//ERROR_Please_enable_LOAD_FONT8_in_User_Setup!
#endif

#ifndef LOAD_GFXFF
ERROR_Please_enable_LOAD_GFXFF_in_User_Setup!
#endif