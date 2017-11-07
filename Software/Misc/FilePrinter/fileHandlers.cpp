#include <Arduino.h>

#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "fileHandlers.h"

//class LogDumper

LogDumper::LogDumper( void ){
	pathStub[0] = 0;
	nameStub[0] = 0;
	fullPath[0] = 0;
	fileNumber = 0;
	buffer[0] = 0;
	bufferPtr = 0;
}

LogDumper::~LogDumper( void ){
}

//	~LogDumper( void ){};
void LogDumper::setDevice( fs::FS * inputDevice )
{
	fs = inputDevice;
}

void LogDumper::setPath( const char * inputString )
{
	sprintf(pathStub, inputString);
	Serial.print("pathStub: ");
	Serial.println(pathStub);
}

void LogDumper::setFileNameStub( const char * inputString )
{
	sprintf(nameStub, inputString);
	Serial.print("nameStub: ");
	Serial.println(nameStub);

	//sprintf(fullPath, "/%s/%s%004d.txt", pathStub, nameStub, fileNumber);
	//Serial.print("fullPath: ");
	//Serial.println(fullPath);

}

int LogDumper::startLog( void )
{
	//Check for log directory
	char s[20];
	sprintf(s, "/%s", pathStub);
	if(isDir(s) == 0)
	{
		Serial.println("no /log dirctory, creating");
		Serial.printf("Creating Dir: %s\n", s);
		if(fs->mkdir(s)){
			Serial.println("Dir created");
		} else {
			Serial.println("mkdir failed");
		}
	}
	else
	{
		Serial.print(s);
		Serial.println(" directory found.");
	}
	//Find last file and make a new one afterwards
	indexNewLogFile();
}

//	int stopLog( void );

int LogDumper::print( const char * inputString )
{
	//Put the inputString into the buffer
	for(int i = 0; (inputString[i] != 0)&&(i < 256); i++)
	{
		if(bufferPtr < 257)
		{
			buffer[bufferPtr] = inputString[i];
			bufferPtr++;
		}
	}
   	if(bufferPtr > 100)
	{
		buffer[bufferPtr] = 0;
		appendFile(buffer);
		//Serial.println(buffer);
		bufferPtr = 0;
		if(getFileSize(fullPath) > 250000)
		{
			//Start new file
			indexNewLogFile();
		}
	}
}

void LogDumper::appendFile( const char * inputString )
{
    Serial.printf("Appending to file: %s\n", fullPath);

    File file = fs->open(fullPath, FILE_APPEND);
    if(!file){
        Serial.println("Failed to open file for appending");
        return;
    }
	else
	{
		if(file.print(inputString)){
			Serial.println("Message appended");
			} else {
			Serial.println("Append failed");
		}
		file.close();
    }
}

float LogDumper::getFileSize(const char * dirname){
    Serial.printf("Listing directory: %s\n", dirname);

    File file = fs->open(dirname);
    if(!file){
        Serial.println("Failed to open file");
        return -1;
    }
	else
	{
		file.close();
		Serial.print("File size: ");
		Serial.println(file.size());
		return file.size();
	}
}

int LogDumper::isDir(const char * dirname){
    Serial.printf("Checking for: %s\n", dirname);

    File root = fs->open(dirname);

    if(!root){
        Serial.println("Failed to open directory");
        return 0;
    }
    if(!root.isDirectory()){
        Serial.println("Not a directory");
        return 0;
    }
	else
	{
		Serial.println("Directory exists");
		root.close();
		return 1;
	}
}

int LogDumper::isFile(const char * dirname){

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

void LogDumper::indexNewLogFile( void )
{
	sprintf(fullPath, "/%s/%s%004d.txt", pathStub, nameStub, fileNumber);
	while(isFile(fullPath))
	{
		Serial.print("file: ");
		Serial.print(fullPath);
		Serial.println(" found.");
		//Index next name
		fileNumber++;
		sprintf(fullPath, "/%s/%s%004d.txt", pathStub, nameStub, fileNumber);
	}
	Serial.println();
	Serial.print("Creating: ");
	Serial.println(fullPath);
	
	
    Serial.printf("Writing file: %s\n", fullPath);

    File file = fs->open(fullPath, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.print("File Created\n\r")){
        Serial.println("File written");
		file.close();
    } else {
        Serial.println("Write failed");
    }
}

void LogDumper::test( void )
{
//	char dirname[10];
//	dirname[0] = '/';
//	dirname[1] = 'l';
//	dirname[2] = 'o';
//	dirname[3] = 'g';
//	dirname[4] = 0;
//	
//    Serial.printf("Listing directory: %s\n", dirname);
//
//    File root = fs->open(dirname);
//    if(!root){
//        Serial.println("Failed to open directory");
//        return;
//    }
//    if(!root.isDirectory()){
//        Serial.println("Not a directory");
//        return;
//    }
//
//    File file = root.openNextFile();
//    while(file){
//        if(file.isDirectory()){
//            Serial.print("  DIR : ");
//            Serial.println(file.name());
//        } else {
//            Serial.print("  FILE: ");
//            Serial.print(file.name());
//            Serial.print("  SIZE: ");
//            Serial.println(file.size());
//        }
//        file = root.openNextFile();
//    }
}
