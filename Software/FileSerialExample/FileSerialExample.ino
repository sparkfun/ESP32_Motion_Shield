/******************************************************************************
FileSerialExample.ino
Example Serial-like file writer

Marshall Taylor @ SparkFun Electronics
original creation date: Nov 6, 2017
https://github.com/sparkfun/ESP32_Motion_Shield

This example demonstrates usage of the FileSerial library.

The FileSerial libary implements the ESP32 SD_Test functions as a class that acts like a
HardwareSerial device.  It has been modeled from the ESP32 Arduino core's
HardwareSerial class, but takes no input streams from the user.

There are a couple extra functions that aren't normally found in a serial device

	int startLog( const char * inputPath, const char * inputStub );
	int stopLog( void );
	void setMaxFileSize( int32_t inputSize );
	void setWriteBufferSize( uint8_t inputSize );

Construct with an optional serial device address, such as

	FileSerial ExampleFileSet(&Serial);

Doing so logs SD read/write information plus written data to the passed serial port.

Resources:
  FileSerial.h
  FileSerial.cpp

Development environment specifics:
  IDE: Arduino 1.8.2
  ESP32 Arduino core
  
This code is released under the [MIT License](http://opensource.org/licenses/MIT).
Please review the LICENSE.md file included with this example. If you have any questions 
or concerns with licensing, please contact techsupport@sparkfun.com.
Distributed as-is; no warranty is given.
******************************************************************************/

#include <Arduino.h>
#include "FileSerial.h"

//Pass address of serial port to see the file IO debug information
FileSerial ExampleFileSet(&Serial);

//...or don't
//FileSerial ExampleFileSet;

int loopCount = 0;

void setup(){
    Serial.begin(115200);
	
	delay(1000);
	Serial.println("Starting Sketch");
	
	//call begin with device, CS pin, port, frequency, and mount point.
	if(ExampleFileSet.begin(&SD, 33, SPI, 10000000, "/sd") == 0)
	{
		Serial.println("SD begin did not succeed, halting.");
		while(1);
	}
	//File name will be appended with file number, ex: filennnn.txt
	
	//You can set max file size in bytes, set 0 for unchecked.
	//Default is 250kB, range 0, 32 to 1000000000
	ExampleFileSet.setMaxFileSize(10000);
	
	//You can as set buffer size between file writes.
	//Default is 100B, range is 1 to 255B
	ExampleFileSet.setWriteBufferSize(80);
	
	//Start a batch of log files with startLog,
	//pass directory name and file name.
	//
	//If directoy is path, parent directories must exist!
	//"[existing directory]/[new directory]" is valid
	//"[new directory]/[new directory]" is not
	ExampleFileSet.startLog("testFiles", "file");
}

void loop(){
	while(Serial.available())
	{
		char c = Serial.read();
		ExampleFileSet.print(c);
	}
	ExampleFileSet.printf("Loop count: %d\n", loopCount); //Formatting works
	ExampleFileSet.println(2.54321, 3); //standard formatting works
	ExampleFileSet.println(0x2E0A, HEX); //and other types
	loopCount++;
	delay(100);

}