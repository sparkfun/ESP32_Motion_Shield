/*****************************************************************
GPS_IMU_SD_Logger.ino
Collect IMU and GPS data, log to SD

Marshall Taylor @ SparkFun Electronics
original creation date: Nov 6, 2017
https://github.com/sparkfun/ESP32_Motion_Shield

This example gathers IMU and GPS data and logs it to the card.

Hardware:
 *ESP32 Thing
 *ESP32 Motion Shield
 *micro SD card inserted
 *GP-20U7 in JST or GP-735 hand-wired

LEDS:

 *GPIO 5 (Onboard thing): Illuminates at start, turns off if:
   *Card not inserted
   *Mount failed
   *IMU start failed
   *File writing error
 *GPIO 13 (Onboard Motion Shield):
   *Blinks on RX GPS packet without lock
   *Solid on good GPS data

Resources:
  FileSerial.h
  FileSerial.cpp

Development environment specifics:
  IDE: Arduino 1.8.2
  ESP32 Arduino core

This code is beerware. If you see me (or any other SparkFun 
employee) at the local, and you've found our code helpful, 
please buy us a round!

Distributed as-is; no warranty is given.
*****************************************************************/
#include <Arduino.h>
#include "FileSerial.h"
#include <SparkFunLSM9DS1.h>

#define AUX_LED_PIN 13
#define ESP_LED_PIN 5

HardwareSerial Serial1(2);  // UART1/Serial1 pins 16,17

FileSerial GPSLogFile(&Serial);
FileSerial IMULogFile(&Serial);

LSM9DS1 imu;  // Create an LSM9DS1 object

#define LSM9DS1_M	0x1E // Would be 0x1C if SDO_M is LOW
#define LSM9DS1_AG	0x6B // Would be 0x6A if SDO_AG is LOW

#define PRINT_SPEED 250 // 250 ms between prints
static unsigned long lastPrint = 0; // Keep track of print time

// Earth's magnetic field varies by location. Add or subtract 
// a declination to get a more accurate heading. Calculate 
// your's here:
// http://www.ngdc.noaa.gov/geomag-web/#declination
#define DECLINATION -8.58 // Declination (degrees) in Boulder, CO.

//Internal variables
float roll;
float pitch;
float heading;
char csvBuffer[300];


void setup(){
	pinMode(AUX_LED_PIN, OUTPUT);
	digitalWrite(AUX_LED_PIN, 0);
	
	pinMode(ESP_LED_PIN, OUTPUT);
	digitalWrite(ESP_LED_PIN, 1);
	
	delay(500);
	
    Serial.begin(115200);
	Serial1.begin(9600);
	
	//IMU
	// Configure LSM9DS1 library parameters
	imu.settings.device.commInterface = IMU_MODE_I2C;
	imu.settings.device.mAddress = LSM9DS1_M;
	imu.settings.device.agAddress = LSM9DS1_AG;
	imu.settings.mag.scale = 2;
	// The above lines will only take effect AFTER calling
	// imu.begin(), which verifies communication with the IMU
	// and turns it on.
	
	
	
	delay(1000);
	Serial.println("Starting Sketch");
	
	delay(100);
	if (!imu.begin())
	{
		Serial.println("Failed to communicate with LSM9DS1.");
		Serial.println("Double-check wiring.");
		Serial.println("Default settings in this sketch will " \
						"work for an out of the box LSM9DS1 " \
						"Breakout, but may need to be modified " \
						"if the board jumpers are.");
		digitalWrite(ESP_LED_PIN, 0);
		
		while (1);
	}
  
    if(!SD.begin(33, SPI, 10000000, "/sd")){
        Serial.println("Card Mount Failed");
		digitalWrite(ESP_LED_PIN, 0);
		while (1);
        return;
    }
    uint8_t cardType = SD.cardType();
	
    if(cardType == CARD_NONE){
        Serial.println("No SD card attached");
		digitalWrite(ESP_LED_PIN, 0);
		while (1);
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

	//call begin with device, CS pin, port, frequency, and mount point.
	if(GPSLogFile.begin(&SD, 33, SPI, 10000000, "/sd") == 0)
	{
		Serial.println("SD begin did not succeed, halting.");
		digitalWrite(ESP_LED_PIN, 0);
		while(1);
	}

	//call begin with device, CS pin, port, frequency, and mount point.
	if(IMULogFile.begin(&SD, 33, SPI, 10000000, "/sd") == 0)
	{
		Serial.println("SD begin did not succeed, halting.");
		digitalWrite(ESP_LED_PIN, 0);
		while(1);
	}	

	GPSLogFile.setMaxFileSize(10000);
	GPSLogFile.setWriteBufferSize(80);
	GPSLogFile.startLog("logFiles", "GPS_");

	IMULogFile.setMaxFileSize(10000);
	IMULogFile.setWriteBufferSize(80);
	IMULogFile.startLog("logFiles", "IMU_");

}

int parsingState = 0;
char parseBuffer[200];
int parseBufferPtr = 0;


void loop(){
	//Pass gps data to the SD card
	switch(parsingState)
	{
		case 0: //reset buffer
			parseBuffer[0] = '$';
			parseBufferPtr = 1;
			parsingState = 1;
		break;
		case 1:
			if(Serial1.available())
			{
				char c = Serial1.read();
				if(c == '$')
				{
					//detected end
					parseBuffer[parseBufferPtr] = 0;
					parseBufferPtr++;
					//Pick out good messsage
					if(( parseBuffer[1] == 'G')&&( parseBuffer[2] == 'P')&&( parseBuffer[3] == 'R')&&( parseBuffer[4] == 'M')&&( parseBuffer[5] == 'C'))
					{
						GPSLogFile.print(parseBuffer);
						//Output data to terminal for debug
						//Serial.print(parseBuffer);
						//
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
				else
				{
					parseBuffer[parseBufferPtr] = c;
					parseBufferPtr++;
				}					
			}
		break;
		default:
		break;
	}
	if ((lastPrint + PRINT_SPEED) < millis())
	{
		if ( imu.gyroAvailable() ) imu.readGyro();
		if ( imu.accelAvailable() ) imu.readAccel();
		if ( imu.magAvailable() ) imu.readMag();
		// Print the heading and orientation for fun!
		// Call print attitude. The LSM9DS1's mag x and y
		// axes are opposite to the accelerometer, so my, mx are
		// substituted for each other.
		calcAttitude(imu.ax, imu.ay, imu.az, -imu.my, -imu.mx, imu.mz);
		float data1 = imu.calcGyro(imu.gx);
		float data2 = imu.calcGyro(imu.gy);
		float data3 = imu.calcGyro(imu.gz);
		float data4 = imu.calcAccel(imu.ax);
		float data5 = imu.calcAccel(imu.ay);
		float data6 = imu.calcAccel(imu.az);
		float data7 = imu.calcMag(imu.mx);
		float data8 = imu.calcMag(imu.my);
		float data9 = imu.calcMag(imu.mz);
		
		sprintf(csvBuffer, "%4.3f,%4.3f,%4.3f,%4.3f,%4.3f,%4.3f,%4.3f,%4.3f,%4.3f,%4.3f,%4.3f,%4.3f,", data1, data2, data3, data4, data5, data6, data7, data8, data9, roll, pitch, heading);
		//Output data to terminal for debug
		//Serial.print(csvBuffer);
		//Serial.println();
		//
		IMULogFile.println(csvBuffer);
//		IMULogFile.print(0x0D);
//		IMULogFile.print(0x0A);
		
		lastPrint = millis(); // Update lastPrint time
	}
}

void calcAttitude(float ax, float ay, float az, float mx, float my, float mz)
{
	roll = atan2(ay, az);
	pitch = atan2(-ax, sqrt(ay * ay + az * az));
	heading;
	
	if (my == 0)
		heading = (mx < 0) ? PI : 0;
	else
		heading = atan2(mx, my);
	
	heading -= DECLINATION * PI / 180;
	
	if (heading > PI) heading -= (2 * PI);
	else if (heading < -PI) heading += (2 * PI);
	else if (heading < 0) heading += 2 * PI;
	
	// Convert everything from radians to degrees:
	heading *= 180.0 / PI;
	pitch *= 180.0 / PI;
	roll  *= 180.0 / PI;
}