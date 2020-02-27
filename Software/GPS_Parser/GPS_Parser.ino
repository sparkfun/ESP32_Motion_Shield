/******************************************************************************
GPS_Parser.h
Simple delimiter and valid data detector

Marshall Taylor @ SparkFun Electronics
original creation date: Nov 6, 2017
https://github.com/sparkfun/ESP32_Motion_Shield

This example checks incomming NMEA data for valid data from satellites.

Resources:
ESP32 Arduino core

Development environment specifics:
Arduino 1.8.2

This code is released under the [MIT License](http://opensource.org/licenses/MIT).
Please review the LICENSE.md file included with this example. If you have any questions 
or concerns with licensing, please contact techsupport@sparkfun.com.
Distributed as-is; no warranty is given.
******************************************************************************/
#include <Arduino.h>

#define AUX_LED_PIN 13

HardwareSerial GPSUART(2);  // Set up GPS UART on pins 16 & 17

#define PRINT_SPEED 250 // 250 ms between prints
static unsigned long lastPrint = 0; // Keep track of print time

void setup(){
	pinMode(AUX_LED_PIN, OUTPUT);
	digitalWrite(AUX_LED_PIN, 0);
	
    Serial.begin(115200);
	GPSUART.begin(9600);
}

int parsingState = 0;
char parseBuffer[200];
int parseBufferPtr = 0;

void loop(){
	//Parse GPS data
	switch(parsingState)
	{
		case 0: //reset buffer
			parseBuffer[0] = '$'; //Pre-load buffer with first char
			parseBufferPtr = 1;
			parsingState = 1;
		break;
		case 1:
			if(GPSUART.available())
			{
				char c = GPSUART.read();
				if(c == '$') //Is char the first char?
				{
					//detected end
					parseBuffer[parseBufferPtr] = 0x0D;
					parseBufferPtr++;
					parseBuffer[parseBufferPtr] = 0x0A;
					parseBufferPtr++;
					parseBuffer[parseBufferPtr] = 0;
					parseBufferPtr++;
					//Pick out good messsage
					if(( parseBuffer[1] == 'G')&&( parseBuffer[2] == 'P')&&( parseBuffer[3] == 'R')&&( parseBuffer[4] == 'M')&&( parseBuffer[5] == 'C'))
					{
						//Output data to terminal for debug
						Serial.print(parseBuffer);
						
						digitalWrite(AUX_LED_PIN, 1);
						delay(1);
						int seekPtr = 0;
						while((parseBuffer[seekPtr] != ',')&&(seekPtr < 25))
						{
							seekPtr++;
						}
						seekPtr++;
						while((parseBuffer[seekPtr] != ',')&&(seekPtr < 25))
						{
							seekPtr++;
						}
						seekPtr++;
						if(parseBuffer[seekPtr] == 'A')
						{
							//digitalWrite(AUX_LED_PIN, 1);
						}
						else
						{
							digitalWrite(AUX_LED_PIN, 0);
						}
						
					}
					parsingState = 0;
				}
				else //Char is not the first char
				{
					parseBuffer[parseBufferPtr] = c;
					parseBufferPtr++;
				}					
			}
		break;
		default:
		break;
	}
}
