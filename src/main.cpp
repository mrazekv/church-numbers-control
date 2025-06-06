#include <Arduino.h>

// Graphics and font library
#include <TFT_eSPI.h>

#include "FreeFont.h"
#include <SPI.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

#include <HTTPClient.h>
#include <Keypad.h>

WiFiManager wm; // global wm instance
WiFiManagerParameter custom_field("url", "URL", "http://example.com", 120); // global param ( for non blocking w params )

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

bool isHold[LIST_MAX] = {0}; // Array to store previous states of keys

#define MEMORY_SIZE 4
int currentNumber = 0;
int memoryStack[MEMORY_SIZE] = {0, 100, 0, 0}; // Stack to store numbers
int currentMemoryIndex = 0; // Current index in the memory stack
int status = 0;

void printShortMessage(const String &message) {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);
    tft.setTextFont(1); // Use a smaller font
    tft.setCursor(10, 10);
    tft.println(message);
    vTaskDelay(4000 / portTICK_PERIOD_MS); // Display for 2 seconds
}

void sendData() {
    Serial.print("Sending data: ");
    Serial.println(currentNumber);
    // Here you can add code to send the data to a server or another device

    status = currentNumber; // Update status with the current number
    // send http request to the server
    String url = custom_field.getValue(); // Get the URL from the custom field
    if (url.length() > 0) {
    
        char buffer[20];
        snprintf(buffer, sizeof(buffer), "%05d", currentNumber); // Convert currentNumber to string
        String url_set =  url + String("set_") + String(buffer); // Append the part number to the URL
        Serial.print("Sending HTTP request to: ");
        Serial.println(url_set);
        // Here you can add code to send the HTTP request using WiFiClient or HTTPClient

        HTTPClient http;
        http.begin(url_set); // Specify the URL
        int httpResponseCode = http.GET();
        if (httpResponseCode > 0) {
             String response = http.getString();
             Serial.println("Response: " + response);
        } else {
             Serial.println("Error on HTTP request: " + String(httpResponseCode));
             printShortMessage("HTTP Error: " + String(httpResponseCode));
        }
        http.end();


        // get status by calling /get on the server
        HTTPClient statusHttp;
        statusHttp.begin(url + "/get"); // Specify the status URL
        int statusHttpResponseCode = statusHttp.GET();
        if (statusHttpResponseCode > 0) {
             String statusResponse = statusHttp.getString();
             Serial.println("Status Response: " + statusResponse);  
            // set status as statusResponse.toInt(); // Convert the response to an integer
            status = statusResponse.toInt();
        } else {
             Serial.println("Error on Status HTTP request: " + String(statusHttpResponseCode));
             printShortMessage("Status HTTP Error: " + String(statusHttpResponseCode));
        }
        statusHttp.end();


    }

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

    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);
    tft.setTextFont(1); // Set text font to 2 (default is 1)
    tft.setCursor(0, 0);

    Serial.begin(9600);

    // wm add parameter "url" to the config portal
    wm.addParameter(&custom_field); // Add the custom parameter to the WiFiManager

    // set timeout for 30 seconds
    wm.setTimeout(30);

    tft.println("Starting WiFiManager... with limit 30 seconds");

    wifiRes = wm.autoConnect(); // auto generated AP name from chipid
    // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
    //wifiRes = wm.autoConnect("AutoConnectAP","password"); // password protected ap

    if(!wifiRes) {
        Serial.println("Failed to connect");
        tft.println("Failed to connect to WiFi");
        tft.println("Restarting...");
        delay(2000);
        ESP.restart();
    } 
    else {
        //if you get here you have connected to the WiFi    
        Serial.println("connected...yeey :)");
        tft.println("Connected to WiFi");
        tft.print("IP address: ");
        tft.println(WiFi.localIP()); // Print the local IP address
    }

    // print the custom parameter value
    Serial.print("Custom URL: ");
    Serial.println(custom_field.getValue());
    tft.print("Custom URL: ");
    tft.println(custom_field.getValue());
    tft.print("RSSI: ");
    tft.println(WiFi.RSSI()); // Print the RSSI value

    delay(2000); // Wait for a second to let the display update


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
}


#ifndef LOAD_GFXFF
ERROR_Please_enable_LOAD_GFXFF_in_User_Setup !
#endif