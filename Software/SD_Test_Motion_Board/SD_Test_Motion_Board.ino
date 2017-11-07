/*****************************************************************
LSM9DS1_CSV.ino

This is a modified version of the SD_Test example sketch
included with the Arduino core for the esp32, from
https://github.com/espressif/arduino-esp32

This example:
 * Uses the CD pin to check for a card
 * Mounts the card
 * Prints information about the card.
 
Use this for a starting place while working with SD cards.

Hardware requirements:
ESP32 Thing attached to Motion Board

Distributed as-is; no warranty is given.
*****************************************************************/
#include "FS.h"
#include "SD.h"
#include "SPI.h"

#define SD_CS_PIN 33
#define SD_CD_PIN 38


void setup(){
    Serial.begin(115200);

	//Check for card presence using CD pin
	pinMode(SD_CD_PIN, INPUT);
	if(digitalRead(SD_CD_PIN) == 1) {
		Serial.println("Card detected");
	} else {
		Serial.println("Card not present");
		return;
	}	
	//Call begin with (cs pin, SPI, rate (up to 10MHz), "/sd")
    if(!SD.begin(SD_CS_PIN, SPI, 1000000, "/sd")){
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

}

void loop(){

}