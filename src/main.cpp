#include <Arduino.h>

// Graphics and font library
#include <TFT_eSPI.h>

#include "FreeFont.h"
#include <SPI.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

#include <Keypad.h>

WiFiManager wm; // global wm instance
WiFiManagerParameter custom_field("url", "URL", "http://example.com", 60); // global param ( for non blocking w params )

const byte ROWS = 4; // four rows
const byte COLS = 3; // three columns
char keys[ROWS][COLS] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}};

// 1    2    3    4    5    6    7    8    9
// G35, G32, G33, G25, G26, G27, G14, G12
// 3, 8, 7, 5
byte rowPins[ROWS] = {33, 12, 14, 26}; // connect to the row pinouts of the kpd
// 4, 2, 6
byte colPins[COLS] = {25, 32, 27}; // connect to the column pinouts of the kpd

Keypad kpd = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

TFT_eSPI tft = TFT_eSPI(); // Invoke library
void header(const char *string, uint16_t color);

// Draw a + mark centred on x,y
void drawDatumMarker(int x, int y);

// put function declarations here:
int myFunction(int, int);

bool isHold[LIST_MAX] = {0}; // Array to store previous states of keys

#define MEMORY_SIZE 4
int currentNumber = 0;
int memoryStack[MEMORY_SIZE] = {0, 100, 0, 0}; // Stack to store numbers
int currentMemoryIndex = 0; // Current index in the memory stack
int status = 0;
void sendData() {
    Serial.print("Sending data: ");
    Serial.println(currentNumber);
    // Here you can add code to send the data to a server or another device

    status = currentNumber; // Update status with the current number

}

// semaphore for display update
SemaphoreHandle_t xSemaphore = xSemaphoreCreateBinary();

void handleKeyPress(char keyChar)
{
    Serial.print("Key pressed: ");
    Serial.println(keyChar);

    if(keyChar >= '0' && keyChar <= '9') {
        currentNumber = currentNumber * 10 + (keyChar - '0'); // Build the number from digits
        // limit to 5 digits
        currentNumber = currentNumber % 100000; // Keep only the last 5 digits
        memoryStack[currentMemoryIndex] = currentNumber; // Store in memory stack
    }

    if (keyChar == '*') { // add part number
        if (currentNumber % 100 == 0) { // If the number is a multiple of 100, reset last two digits
            currentNumber = currentNumber / 100 * 100; // Reset last two digits to zero
        } else if (currentNumber % 100 < 99) { // If not, increment by 1
            currentNumber += 1;
        }
        memoryStack[currentMemoryIndex] = currentNumber; // Store in memory stack
        sendData(); // Call the function to send data
    }

    if (keyChar == '#') {
        sendData(); // Call the function to send data
    }

    // alert main task to update display
    xSemaphoreGive(xSemaphore);

}

void handleKeyHold(char keyChar)
{
    // 1 to MEMORY SIZE switch memory
    if(keyChar >= '1' && keyChar <= '0' + MEMORY_SIZE) {
        int index = keyChar - '1'; // Convert char to index (0-3)
        if (index < 0 || index >= MEMORY_SIZE) {
            Serial.println("Invalid memory slot");
            return;
        }
        currentMemoryIndex = index; // Switch to the selected memory slot
        currentNumber = memoryStack[currentMemoryIndex]; // Load the number from memory
        Serial.print("Switched to memory slot ");
        Serial.println(index + 1);
        xSemaphoreGive(xSemaphore); // Alert the main task to update display
    }

    else if(keyChar == '*') { // decrement part
        if(currentNumber % 100 > 0) {
            currentNumber -= 1;
            memoryStack[currentMemoryIndex] = currentNumber; // Store in memory stack
        }
        xSemaphoreGive(xSemaphore); // Alert the main task to update display
        sendData(); // Call the function to send data
    }

    else if(keyChar == '0') { // add two zeros (multply by 100)
        currentNumber *= 100;
        currentNumber = currentNumber % 100000; // Keep only the last 5 digits
        memoryStack[currentMemoryIndex] = currentNumber; // Store in memory stack
        xSemaphoreGive(xSemaphore); // Alert the main task to update display
    }

    else if(keyChar == '#') { // send data
        sendData(); // Call the function to send data
        xSemaphoreGive(xSemaphore); // Alert the main task to update display
    }

    Serial.print("Key held: ");
    Serial.println(keyChar);
}

void keyboardTask(void *pvParameters)
{
    while (1)
    {
        // Fills kpd.key[ ] array with up-to 10 active keys.
        // Returns true if there are ANY active keys.
        if (kpd.getKeys())
        {
            for (int i = 0; i < LIST_MAX; i++) // Scan the whole key list.
            {
                if (kpd.key[i].stateChanged) // Only find keys that have changed state.
                {
                    switch (kpd.key[i].kstate)
                    { // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
                    case PRESSED:
                        isHold[i] = false; // Reset hold state when key is pressed
                        break;
                    case HOLD:
                        isHold[i] = true; // Set hold state when key is held down
                        handleKeyHold(kpd.key[i].kchar);
                        break;
                    case RELEASED:
                        if (!isHold[i])
                        { // Only print if it was not a hold
                            handleKeyPress(kpd.key[i].kchar);
                        }
                        break;
                    case IDLE:
                        break;
                    }
                }
            }
        }

        vTaskDelay(100 / portTICK_PERIOD_MS); // Delay to avoid flooding the serial output
    }
}

bool wifiRes;

void setup()
{
    Serial.begin(9600);

    // wm add parameter "url" to the config portal
    wm.addParameter(&custom_field); // Add the custom parameter to the WiFiManager



    //wifiRes = wm.autoConnect(); // auto generated AP name from chipid
    // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
    //wifiRes = wm.autoConnect("AutoConnectAP","password"); // password protected ap

    if(!wifiRes) {
        Serial.println("Failed to connect");
        // ESP.restart();
    } 
    else {
        //if you get here you have connected to the WiFi    
        Serial.println("connected...yeey :)");
    }

    // print the custom parameter value
    Serial.print("Custom URL: ");
    Serial.println(custom_field.getValue());


    // put your setup code here, to run once:
    int result = 5;
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

    // create a task to handle the keyboard input
    xTaskCreate(
        keyboardTask,   // Function to implement the task
        "KeyboardTask", // Name of the task
        10000,          // Stack size in words
        NULL,           // Task input parameter
        1,              // Priority of the task
        NULL);          // Task handle
}

void drawMemoryElement(int index, int x, int y) {
    static char buffer[20];
    bool isCurrent = (index == currentMemoryIndex);
    
    tft.setTextColor(TFT_SILVER, TFT_BLACK);
    // left align text
    if (isCurrent) {
        tft.fillRoundRect(x, y, 60, 20, 5, TFT_BLUE);
        tft.setTextColor(TFT_YELLOW, TFT_BLUE); // Highlight current memory element
    } else {
        tft.drawRoundRect(x, y, 60, 20, 5, TFT_SILVER);
        tft.setTextColor(TFT_SILVER, TFT_BLACK);
    }
    tft.setTextDatum(TL_DATUM); // Top left datum
    tft.setFreeFont(FM9);        // Select Free Mono 9 point font
    snprintf(buffer, sizeof(buffer), "%d:%05d", index + 1, memoryStack[index]); // Format the string with index and value
    tft.drawString(buffer, x + 5, y + 3, 2); // Draw the text string in the selected GFX free font

}

void loop()
{

    Serial.println("loop1");
    Serial.print("Current number: ");
    Serial.println(currentNumber);
    Serial.print("Current memory index: ");
    Serial.println(currentMemoryIndex);
    Serial.print("Memory stack: ");
    for (int i = 0; i < MEMORY_SIZE; i++)
    {
        Serial.print(memoryStack[i]);
        if (i < MEMORY_SIZE - 1)
            Serial.print(", ");
    }
    Serial.println();


    char buffer[50];

    int xpos = 0;
    int ypos = 40;
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0);
    tft.setTextSize(1);


    snprintf(buffer, sizeof(buffer), "Status: %05d", status);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setFreeFont(&FreeMono9pt7b); // Select Free Serif 18 point font
    // align right
    tft.setTextDatum(TR_DATUM); // Top right datum
    tft.drawString(buffer, tft.width() - 10, 10, 2); // Draw the text string in the selected GFX free font


    tft.setFreeFont(FSSB24);    // Select Free Serif 24 point font
    tft.setTextDatum(TC_DATUM); // Centre text on x,y position

    // safe sprintf
    snprintf(buffer, sizeof(buffer), "%03d", currentNumber / 100);

    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawString(buffer, 50, 30, GFXFF); // Draw the text string in the selected GFX free font

    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.setFreeFont(FSS24);
    snprintf(buffer, sizeof(buffer), "%02d", (currentNumber) % 100);
    tft.drawString(buffer, 125, 30 + 3, GFXFF); // Draw the text string in the selected GFX free font


    drawMemoryElement(0, 15, 82); // Draw memory element 1
    drawMemoryElement(1, 85, 82); // Draw memory element 2
    drawMemoryElement(2, 15, 105); // Draw memory element 3
    drawMemoryElement(3, 85, 105); // Draw memory element 4

    // wait for update
    xSemaphoreTake(xSemaphore, portMAX_DELAY);
    return;
    // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    // Select different fonts to draw on screen using the print class
    // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

    header("Using print() method", TFT_NAVY);

    // For comaptibility with Adafruit_GFX library the text background is not plotted when using the print class
    // even if we specify it.
    tft.setTextColor(TFT_YELLOW);
    tft.setCursor(xpos, ypos); // Set cursor near top left corner of screen

    tft.setFreeFont(TT1);                       // Select the orginal small TomThumb font
    tft.println();                              // Move cursor down a line
    tft.print("The really tiny TomThumb font"); // Print the font name onto the TFT screen
    tft.println();
    tft.println();

    tft.setFreeFont(FSB9); // Select Free Serif 9 point font, could use:
    // tft.setFreeFont(&FreeSerif9pt7b);
    tft.println(); // Free fonts plot with the baseline (imaginary line the letter A would sit on)
    // as the datum, so we must move the cursor down a line from the 0,0 position
    tft.print("Serif Bold 9pt"); // Print the font name onto the TFT screen

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

    tft.setFreeFont(FSB9);                               // Select the font
    tft.drawString("Serif Bold 9pt", xpos, ypos, GFXFF); // Draw the text string in the selected GFX free font
    ypos += tft.fontHeight(GFXFF);                       // Get the font height and move ypos down

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
    for (int i = 0; i <= 20; i++)
    {
        tft.drawFloat(i / 10.0, 1, xpos, ypos, GFXFF);
        delay(200);
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

    tft.setFreeFont(FSB9);                               // Select the font
    tft.drawString("Serif Bold 9pt", xpos, ypos, GFXFF); // Draw the text string in the selected GFX free font
    ypos += tft.fontHeight(GFXFF);                       // Get the font height and move ypos down

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
    for (int i = 0; i <= 20; i++)
    {
        tft.drawFloat(i / 10.0, 1, xpos, ypos, GFXFF);
        delay(200);
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
    drawDatumMarker(160, 120);
    delay(1000);

    tft.fillRect(0, 80, 320, 80, TFT_BLACK);
    tft.setTextDatum(TC_DATUM);
    tft.drawString("[Top centre]", 160, 120, GFXFF);
    drawDatumMarker(160, 120);
    delay(1000);

    tft.fillRect(0, 80, 320, 80, TFT_BLACK);
    tft.setTextDatum(TR_DATUM);
    tft.drawString("[Top right]", 160, 120, GFXFF);
    drawDatumMarker(160, 120);
    delay(1000);

    tft.fillRect(0, 80, 320, 80, TFT_BLACK);
    tft.setTextDatum(ML_DATUM);
    tft.drawString("[Middle left]", 160, 120, GFXFF);
    drawDatumMarker(160, 120);
    delay(1000);

    tft.fillRect(0, 80, 320, 80, TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("[Middle centre]", 160, 120, GFXFF);
    drawDatumMarker(160, 120);
    delay(1000);

    tft.fillRect(0, 80, 320, 80, TFT_BLACK);
    tft.setTextDatum(MR_DATUM);
    tft.drawString("[Middle right]", 160, 120, GFXFF);
    drawDatumMarker(160, 120);
    delay(1000);

    tft.fillRect(0, 80, 320, 80, TFT_BLACK);
    tft.setTextDatum(BL_DATUM);
    tft.drawString("[Bottom left]", 160, 120, GFXFF);
    drawDatumMarker(160, 120);
    delay(1000);

    tft.fillRect(0, 80, 320, 80, TFT_BLACK);
    tft.setTextDatum(BC_DATUM);
    tft.drawString("[Bottom centre]", 160, 120, GFXFF);
    drawDatumMarker(160, 120);
    delay(1000);

    tft.fillRect(0, 80, 320, 80, TFT_BLACK);
    tft.setTextDatum(BR_DATUM);
    tft.drawString("[Bottom right]", 160, 120, GFXFF);
    drawDatumMarker(160, 120);
    delay(1000);

    tft.fillRect(0, 80, 320, 80, TFT_BLACK);
    tft.setTextDatum(L_BASELINE);
    tft.drawString("[Left baseline]", 160, 120, GFXFF);
    drawDatumMarker(160, 120);
    delay(1000);

    tft.fillRect(0, 80, 320, 80, TFT_BLACK);
    tft.setTextDatum(C_BASELINE);
    tft.drawString("[Centre baseline]", 160, 120, GFXFF);
    drawDatumMarker(160, 120);
    delay(1000);

    tft.fillRect(0, 80, 320, 80, TFT_BLACK);
    tft.setTextDatum(R_BASELINE);
    tft.drawString("[Right baseline]", 160, 120, GFXFF);
    drawDatumMarker(160, 120);
    delay(1000);

    // while(1);
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
// ERROR_Please_enable_LOAD_GLCD_in_User_Setup
#endif

#ifndef LOAD_FONT2
// ERROR_Please_enable_LOAD_FONT2_in_User_Setup!
#endif

#ifndef LOAD_FONT4
// ERROR_Please_enable_LOAD_FONT4_in_User_Setup!
#endif

#ifndef LOAD_FONT6
// ERROR_Please_enable_LOAD_FONT6_in_User_Setup!
#endif

#ifndef LOAD_FONT7
// ERROR_Please_enable_LOAD_FONT7_in_User_Setup!
#endif

#ifndef LOAD_FONT8
// ERROR_Please_enable_LOAD_FONT8_in_User_Setup!
#endif

#ifndef LOAD_GFXFF
ERROR_Please_enable_LOAD_GFXFF_in_User_Setup !
#endif