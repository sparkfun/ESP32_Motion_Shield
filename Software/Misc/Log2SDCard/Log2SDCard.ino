#include "SDHelpers.h"
#include <Arduino.h>
HardwareSerial Serial1(2);  // UART1/Serial1 pins 16,17

char logPath[20];
char serialBuffer[100];
int serialBufferPtr;
int seekNum;

void indexNewLogFile( void )
{
	sprintf(logPath, "/log/log%004d.txt", seekNum);
	while(isFile(SD, logPath))
	{
		Serial.print("file: ");
		Serial.print(logPath);
		Serial.println(" found.");
		//Index next name
		seekNum++;
		sprintf(logPath, "/log/log%004d.txt", seekNum);
	}
	Serial.println();
	Serial.print("Creating: ");
	Serial.println(logPath);
	writeFile(SD, logPath, "File created\n");
}

void setup(){
    Serial.begin(115200);
	Serial1.begin(9600);
	serialBufferPtr = 0;
	
    if(!SD.begin(33, SPI, 10000000, "/sd")){
        Serial.println("Card Mount Failed");
        return;
    }
    uint8_t cardType = SD.cardType();
	
    if(cardType == CARD_NONE){
        Serial.println("No SD card attached");
        return;
    }

    Serial.print("SD Card Type: ");
    if(cardType == CARD_MMC){
        Serial.println("MMC");
    } else if(cardType == CARD_SD){
        Serial.println("SDSC");
    } else if(cardType == CARD_SDHC){
        Serial.println("SDHC");
    } else {
        Serial.println("UNKNOWN");
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);

	//Check for log directory
	if(isDir(SD, "/log") == 0)
	{
		Serial.println("no /log dirctory, creating");
		createDir(SD, "/log");
	}
	else
	{
		Serial.println("/log directory found.");
	}
	
	//Find last enumerated file in format "lognnnn.txt"
	seekNum = 0;

	indexNewLogFile();
}

void loop(){
	//Pass gps data to the SD card
	if (Serial1.available())
	{
		char c = Serial1.read();
		serialBuffer[serialBufferPtr] = c;
		serialBufferPtr++;
	}
	if(serialBufferPtr > 80)
	{
		serialBuffer[serialBufferPtr] = 0;
		appendFile(SD, logPath, serialBuffer);
		//Serial.println(serialBuffer);
		serialBufferPtr = 0;
		if(getFileSize(SD, logPath) > 250000)
		{
			//Start new file
			indexNewLogFile();
		}
	}
}
