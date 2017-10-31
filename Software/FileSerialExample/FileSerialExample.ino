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