#include <Arduino.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "FileSerial.h"


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
}

//void FileSerial::begin(unsigned long baud, uint32_t config, int8_t rxPin, int8_t txPin)
int FileSerial::begin(fs::FS * inputDevice, uint8_t ssPin, SPIClass &spi, uint32_t frequency, const char * mountpoint)
{
	fs = inputDevice;
	sdCardOpen = 1;
    if(!SD.begin(ssPin, spi, frequency, mountpoint)){
        _SerialDebug->println("Card Mount Failed");
		sdCardOpen = 0;
        return 0;
    }
    uint8_t cardType = SD.cardType();
	
    if(cardType == CARD_NONE){
        _SerialDebug->println("No SD card attached");
		sdCardOpen = 0;
        return 0;
    }

    _SerialDebug->print("SD Card Type: ");
    if(cardType == CARD_MMC){
        _SerialDebug->println("MMC");
    } else if(cardType == CARD_SD){
        _SerialDebug->println("SDSC");
    } else if(cardType == CARD_SDHC){
        _SerialDebug->println("SDHC");
    } else {
        _SerialDebug->println("UNKNOWN");
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    _SerialDebug->printf("SD Card Size: %lluMB\n", cardSize);
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
		_SerialDebug->print((char)c);
		bufferPtr++;
	}
   	if(bufferPtr >= writeBufferSize)
	{
		_SerialDebug->printf("[Writing %d bytes]\n", writeBufferSize);
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
	_SerialDebug->print("pathStub: ");
	_SerialDebug->println(pathStub);

	sprintf(nameStub, inputStub);
	_SerialDebug->print("nameStub: ");
	_SerialDebug->println(nameStub);

	//Check for log directory
	char s[225];
	sprintf(s, "/%s", pathStub);
	if(isDir(s) == 0)
	{
		_SerialDebug->println("no dirctory, creating");
		_SerialDebug->printf("Creating Dir: %s\n", s);
		if(fs->mkdir(s)){
			_SerialDebug->println("Dir created");
		} else {
			_SerialDebug->println("mkdir failed");
		}
	}
	else
	{
		_SerialDebug->print(s);
		_SerialDebug->println(" directory found.");
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
    _SerialDebug->printf("Appending to file: %s\n", fullPath);

    File file = fs->open(fullPath, FILE_APPEND);
    if(!file){
        _SerialDebug->println("Failed to open file for appending");
        return;
    }
	else
	{
		if(file.print(inputString)){
			_SerialDebug->println("Message appended");
			} else {
			_SerialDebug->println("Append failed");
		}
		file.close();
    }
}

float FileSerial::getFileSize(const char * dirname){
    File file = fs->open(dirname);
    if(!file){
        _SerialDebug->println("Failed to open file");
        return -1;
    }
	else
	{
		_SerialDebug->print("File size: ");
		_SerialDebug->println(file.size());
		return file.size();
		file.close();
	}
}

int FileSerial::isDir(const char * dirname){
    _SerialDebug->printf("Checking for: %s\n", dirname);

    File root = fs->open(dirname);

    if(!root){
        _SerialDebug->println("Failed to open directory");
        return 0;
    }
    if(!root.isDirectory()){
        _SerialDebug->println("Not a directory");
        return 0;
    }
	else
	{
		_SerialDebug->println("Directory exists");
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
		_SerialDebug->print("file: ");
		_SerialDebug->print(fullPath);
		_SerialDebug->println(" found.");
		//Index next name
		fileNumber++;
		sprintf(fullPath, "/%s/%s%004d.txt", pathStub, nameStub, fileNumber);
	}
	_SerialDebug->println();
	_SerialDebug->print("Creating: ");
	_SerialDebug->println(fullPath);
	
	
    _SerialDebug->printf("Writing file: %s\n", fullPath);

    File file = fs->open(fullPath, FILE_WRITE);
    if(!file){
        _SerialDebug->println("Failed to open file for writing");
        return;
    }
    if(file.print("File Created\n")){
        _SerialDebug->println("File written");
		file.close();
    } else {
        _SerialDebug->println("Write failed");
    }
}
