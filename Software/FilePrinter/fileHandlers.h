#ifndef FILEHANDLERS_H
#define FILEHANDLERS_H

#include "FS.h"
#include "SD.h"
#include "SPI.h"

class LogDumper
{
public:
	LogDumper( void );
	~LogDumper( void );
	void setDevice( fs::FS * inputDevice );
	void setPath( const char * inputString );
	void setFileNameStub( const char * inputString );
	int startLog( void );
	int stopLog( void );
	int print( const char * inputString );
	void test( void );//Kill this later
private:
	int isDir(const char * dirname);
	int isFile(const char * dirname);
	float getFileSize( const char * dirname);
	void appendFile( const char * inputString );
	void indexNewLogFile( void );
	char pathStub[20];
	char nameStub[20];
	char fullPath[50];
	char buffer[256];
	int bufferPtr;
	int fileNumber;
	fs::FS * fs;
	
};

#endif