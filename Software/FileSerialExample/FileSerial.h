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
	int sdCardOpen;
	int32_t maxFileSize;
	int32_t writeBufferSize;

};


#endif
