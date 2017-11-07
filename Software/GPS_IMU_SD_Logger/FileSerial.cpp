/******************************************************************************
FileSerial.cpp
Serial-like file writer

Marshall Taylor @ SparkFun Electronics
original creation date: Nov 6, 2017
https://github.com/sparkfun/ESP32_Motion_Shield

This library implements the ESP32 SD_Test functions as a class that acts like a
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
ESP32 Arduino core

Development environment specifics:
Arduino 1.8.2

This code is released under the [MIT License](http://opensource.org/licenses/MIT).
Please review the LICENSE.md file included with this example. If you have any questions 
or concerns with licensing, please contact techsupport@sparkfun.com.
Distributed as-is; no warranty is given.
******************************************************************************/

#include <Arduino.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "FileSerial.h"

bool FileSerial::sdCardOpen;


//FileSerial::FileSerial(int uart_nr) : _uart_nr(uart_nr), _uart(NULL) {}
FileSerial::FileSerial( HardwareSerial* hwPort )
{
	maxFileSize = 250000;
	writeBufferSize = 100;
	pathStub[0] = 0;
	nameStub[0] = 0;
	fullPath[0] = 0;
	fileNumber = 0;
	buffer[0] = 0;
	bufferPtr = 0;
	sdCardOpen = 0;
	_SerialDebug = hwPort;
	outputSerial = true;
}

FileSerial::FileSerial( void )
{
	maxFileSize = 250000;
	writeBufferSize = 100;
	pathStub[0] = 0;
	nameStub[0] = 0;
	fullPath[0] = 0;
	fileNumber = 0;
	buffer[0] = 0;
	bufferPtr = 0;
	sdCardOpen = 0;
	_SerialDebug = NULL;
	outputSerial = false;
}

//void FileSerial::begin(unsigned long baud, uint32_t config, int8_t rxPin, int8_t txPin)
int FileSerial::begin(fs::FS * inputDevice, uint8_t ssPin, SPIClass &spi, uint32_t frequency, const char * mountpoint)
{
	fs = inputDevice;
	if( sdCardOpen == 1 ) { //Card is already open from another instance
		if(outputSerial) _SerialDebug->print("Card already mounted.");
		return 1;
	}
	
	sdCardOpen = 1;
    if(!SD.begin(ssPin, spi, frequency, mountpoint)){
        if(outputSerial) _SerialDebug->println("Card Mount Failed");
		sdCardOpen = 0;
        return 0;
    }
    uint8_t cardType = SD.cardType();
	
    if(cardType == CARD_NONE){
        if(outputSerial) _SerialDebug->println("No SD card attached");
		sdCardOpen = 0;
        return 0;
    }

    if(outputSerial) _SerialDebug->print("SD Card Type: ");
    if(cardType == CARD_MMC){
        if(outputSerial) _SerialDebug->println("MMC");
    } else if(cardType == CARD_SD){
        if(outputSerial) _SerialDebug->println("SDSC");
    } else if(cardType == CARD_SDHC){
        if(outputSerial) _SerialDebug->println("SDHC");
    } else {
        if(outputSerial) _SerialDebug->println("UNKNOWN");
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    if(outputSerial) _SerialDebug->printf("SD Card Size: %lluMB\n", cardSize);
	return sdCardOpen;
}

void FileSerial::end()
{
    //
}

void FileSerial::setMaxFileSize( int32_t inputSize )
{
	if( inputSize == 0 )
	{
		maxFileSize = 0;
	}
	else
	{
		if( inputSize < 32 ) inputSize = 32;
		if( inputSize > 1000000000 ) inputSize = 1000000000;
		maxFileSize = inputSize;
	}
}

void FileSerial::setWriteBufferSize( uint8_t inputSize )
{
	if( inputSize < 1 ) inputSize = 1;
	writeBufferSize = inputSize;
	
}


size_t FileSerial::write(uint8_t c)
{
	if(bufferPtr < 257)
	{
		buffer[bufferPtr] = c;
		if(outputSerial) _SerialDebug->print((char)c);
		bufferPtr++;
	}
   	if(bufferPtr >= writeBufferSize)
	{
		if(outputSerial) _SerialDebug->printf("[Writing %d bytes]\n", writeBufferSize);
		buffer[bufferPtr] = 0;
		appendFile(buffer);
		bufferPtr = 0;
		if( maxFileSize != 0 )
		{
			if(getFileSize(fullPath) > maxFileSize)
			{
				//Start new file
				indexNewLogFile();
			}
		}
	}
    return 1;
}

size_t FileSerial::write(const uint8_t *buffer, size_t size)
{
    //uartWriteBuf(_uart, buffer, size);
	while( size > 0 )
	{
		write(*buffer++);
		size--;
	}
    return size;
}

int FileSerial::startLog( const char * inputPath, const char * inputStub )
{
	if( sdCardOpen == 0 ) return 0; //Bail if it the card didn't open.
	
	//Need to check path lengths here!
	sprintf(pathStub, inputPath);
	if(outputSerial) _SerialDebug->print("pathStub: ");
	if(outputSerial) _SerialDebug->println(pathStub);

	sprintf(nameStub, inputStub);
	if(outputSerial) _SerialDebug->print("nameStub: ");
	if(outputSerial) _SerialDebug->println(nameStub);

	//Check for log directory
	char s[225];
	sprintf(s, "/%s", pathStub);
	if(isDir(s) == 0)
	{
		if(outputSerial) _SerialDebug->println("no dirctory, creating");
		if(outputSerial) _SerialDebug->printf("Creating Dir: %s\n", s);
		if(fs->mkdir(s)){
			if(outputSerial) _SerialDebug->println("Dir created");
		} else {
			if(outputSerial) _SerialDebug->println("mkdir failed");
		}
	}
	else
	{
		if(outputSerial) _SerialDebug->print(s);
		if(outputSerial) _SerialDebug->println(" directory found.");
	}

	//Find last file and make a new one afterwards
	indexNewLogFile();
	return 1;
}

int FileSerial::stopLog( void )
{
	return 1;
}
void FileSerial::appendFile( const char * inputString )
{
    if(outputSerial) _SerialDebug->printf("Appending to file: %s\n", fullPath);

    File file = fs->open(fullPath, FILE_APPEND);
    if(!file){
        if(outputSerial) _SerialDebug->println("Failed to open file for appending");
        return;
    }
	else
	{
		if(file.print(inputString)){
			if(outputSerial) _SerialDebug->println("Message appended");
			} else {
			if(outputSerial) _SerialDebug->println("Append failed");
		}
		file.close();
    }
}

float FileSerial::getFileSize(const char * dirname){
    File file = fs->open(dirname);
    if(!file){
        if(outputSerial) _SerialDebug->println("Failed to open file");
        return -1;
    }
	else
	{
		if(outputSerial) _SerialDebug->print("File size: ");
		if(outputSerial) _SerialDebug->println(file.size());
		return file.size();
		file.close();
	}
}

int FileSerial::isDir(const char * dirname){
    if(outputSerial) _SerialDebug->printf("Checking for: %s\n", dirname);
	delay(1000);

    File root = fs->open(dirname);

    if(!root){
        if(outputSerial) _SerialDebug->println("Failed to open directory");
        return 0;
    }
    if(!root.isDirectory()){
        if(outputSerial) _SerialDebug->println("Not a directory");
        return 0;
    }
	else
	{
		if(outputSerial) _SerialDebug->println("Directory exists");
		root.close();
		return 1;
	}
}

int FileSerial::isFile(const char * dirname){

    File file = fs->open(dirname);
    if(!file){
        return 0;
    }
    else
	{
		file.close();
        return 1;
    }
}

void FileSerial::indexNewLogFile( void )
{
	sprintf(fullPath, "/%s/%s%004d.txt", pathStub, nameStub, fileNumber);
	while(isFile(fullPath))
	{
		if(outputSerial) _SerialDebug->print("file: ");
		if(outputSerial) _SerialDebug->print(fullPath);
		if(outputSerial) _SerialDebug->println(" found.");
		//Index next name
		fileNumber++;
		sprintf(fullPath, "/%s/%s%004d.txt", pathStub, nameStub, fileNumber);
	}
	if(outputSerial) _SerialDebug->println();
	if(outputSerial) _SerialDebug->print("Creating: ");
	if(outputSerial) _SerialDebug->println(fullPath);
	
	
    if(outputSerial) _SerialDebug->printf("Writing file: %s\n", fullPath);

    File file = fs->open(fullPath, FILE_WRITE);
    if(!file){
        if(outputSerial) _SerialDebug->println("Failed to open file for writing");
        return;
    }
    if(file.print("File Created\n")){
        if(outputSerial) _SerialDebug->println("File written");
		file.close();
    } else {
        if(outputSerial) _SerialDebug->println("Write failed");
    }
}
