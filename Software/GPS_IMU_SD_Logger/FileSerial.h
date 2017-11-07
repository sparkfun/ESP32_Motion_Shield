/******************************************************************************
FileSerial.h
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
#ifndef FILEHANDLERS_H
#define FILEHANDLERS_H

#include <inttypes.h>
#include "Stream.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"

class FileSerial: public Stream
{
public:
    FileSerial( HardwareSerial* hwPort );
	FileSerial( void );
	//HardwareSerial-like functions
	int begin(fs::FS * inputDevice, uint8_t ssPin, SPIClass &spi, uint32_t frequency, const char * mountpoint);
    void end();
    int available(void){return -1;};
    int peek(void){return -1;};
    int read(void){return -1;};
    void flush(void){};
    size_t write(uint8_t);
    size_t write(const uint8_t *buffer, size_t size);
    inline size_t write(const char * s)
    {
        return write((uint8_t*) s, strlen(s));
    }
    inline size_t write(unsigned long n)
    {
        return write((uint8_t) n);
    }
    inline size_t write(long n)
    {
        return write((uint8_t) n);
    }
    inline size_t write(unsigned int n)
    {
        return write((uint8_t) n);
    }
    inline size_t write(int n)
    {
        return write((uint8_t) n);
    }
	//Logging functions
	int startLog( const char * inputPath, const char * inputStub );
	int stopLog( void );
	void setMaxFileSize( int32_t inputSize );
	void setWriteBufferSize( uint8_t inputSize );
protected:
    //int _uart_nr;
    HardwareSerial* _SerialDebug;
private:
	int isDir(const char * dirname);
	int isFile(const char * dirname);
	float getFileSize( const char * dirname);
	void appendFile( const char * inputString );
	void indexNewLogFile( void );
	char pathStub[100];
	char nameStub[100];
	char fullPath[225];
	char buffer[256];
	int bufferPtr;
	int fileNumber;
	fs::FS * fs;
	static bool sdCardOpen;
	int32_t maxFileSize;
	int32_t writeBufferSize;
	bool outputSerial;

};


#endif
