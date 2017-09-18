
#include <Arduino.h>
#include "fileHandlers.h"

HardwareSerial Serial1(2);  // UART1/Serial1 pins 16,17

LogDumper testFile1;
LogDumper testFile2;

void setup(){
    Serial.begin(115200);
	Serial1.begin(9600);
	
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

	testFile1.setDevice(&SD);
	testFile1.setPath("testPath1");
	testFile1.setFileNameStub("log");
	testFile1.startLog();

	testFile2.setDevice(&SD);
	testFile2.setPath("testPath2");
	testFile2.setFileNameStub("log");
	testFile2.startLog();

}

int iGlobal = 0;
int jGlobal = 0;

void loop(){
	//Pass gps data to the SD card
	if (Serial.available())
	{
		char c[2];
		c[0] = Serial.read();
		c[1] = 0;
		testFile2.print(c);
	}
	iGlobal++;
	jGlobal++;
	delay(1);
	if(iGlobal > 1000)
	{
		iGlobal = 0;
		char s[20];
		sprintf(s, "%d ", jGlobal);
		testFile1.print(s);
	}
}
