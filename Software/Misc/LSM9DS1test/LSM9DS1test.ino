/*****************************************************************
LSM9DS1_Basic_I2C.ino
SFE_LSM9DS1 Library Simple Example Code - I2C Interface
Jim Lindblom @ SparkFun Electronics
Original Creation Date: April 30, 2015
https://github.com/sparkfun/LSM9DS1_Breakout

The LSM9DS1 is a versatile 9DOF sensor. It has a built-in
accelerometer, gyroscope, and magnetometer. Very cool! Plus it
functions over either SPI or I2C.

This Arduino sketch is a demo of the simple side of the
SFE_LSM9DS1 library. It'll demo the following:
* How to create a LSM9DS1 object, using a constructor (global
  variables section).
* How to use the begin() function of the LSM9DS1 class.
* How to read the gyroscope, accelerometer, and magnetometer
  using the readGryo(), readAccel(), readMag() functions and 
  the gx, gy, gz, ax, ay, az, mx, my, and mz variables.
* How to calculate actual acceleration, rotation speed, 
  magnetic field strength using the calcAccel(), calcGyro() 
  and calcMag() functions.
* How to use the data from the LSM9DS1 to calculate 
  orientation and heading.

Hardware setup: This library supports communicating with the
LSM9DS1 over either I2C or SPI. This example demonstrates how
to use I2C. The pin-out is as follows:
	LSM9DS1 --------- Arduino
	 SCL ---------- SCL (A5 on older 'Duinos')
	 SDA ---------- SDA (A4 on older 'Duinos')
	 VDD ------------- 3.3V
	 GND ------------- GND
(CSG, CSXM, SDOG, and SDOXM should all be pulled high. 
Jumpers on the breakout board will do this for you.)

The LSM9DS1 has a maximum voltage of 3.6V. Make sure you power it
off the 3.3V rail! I2C pins are open-drain, so you'll be 
(mostly) safe connecting the LSM9DS1's SCL and SDA pins 
directly to the Arduino.

Development environment specifics:
	IDE: Arduino 1.6.3
	Hardware Platform: SparkFun Redboard
	LSM9DS1 Breakout Version: 1.0

This code is beerware. If you see me (or any other SparkFun 
employee) at the local, and you've found our code helpful, 
please buy us a round!

Distributed as-is; no warranty is given.
*****************************************************************/
// The SFE_LSM9DS1 library requires both Wire and SPI be
// included BEFORE including the 9DS1 library.
#include <Wire.h>
#include <SPI.h>
#include <SparkFunLSM9DS1.h>
#include "WiFi.h"
//////////////////////////
// LSM9DS1 Library Init //
//////////////////////////
// Use the LSM9DS1 class to create an object. [imu] can be
// named anything, we'll refer to that throught the sketch.
LSM9DS1 imu;

///////////////////////
// Example I2C Setup //
///////////////////////
// SDO_XM and SDO_G are both pulled high, so our addresses are:
#define LSM9DS1_M	0x1E // Would be 0x1C if SDO_M is LOW
#define LSM9DS1_AG	0x6B // Would be 0x6A if SDO_AG is LOW

#define LSM9DS1_VDD_CTRL 27
#define LSM9DS1_VDDIO_CTRL 34

////////////////////////////
// Sketch Output Settings //
////////////////////////////
#define PRINT_CALCULATED
//#define PRINT_RAW
#define PRINT_SPEED 250 // 250 ms between prints
#define SCAN_SPEED 1000
static unsigned long lastPrint = 0; // Keep track of print time
static unsigned long lastScan = 0; // Keep track of print time

// Earth's magnetic field varies by location. Add or subtract 
// a declination to get a more accurate heading. Calculate 
// your's here:
// http://www.ngdc.noaa.gov/geomag-web/#declination
#define DECLINATION -8.58 // Declination (degrees) in Boulder, CO.

#define NUMPAIRS 10
class wifiTable
{
public:
	wifiTable(void)
	{
		for( int i = 0; i < NUMPAIRS; i++ )
		{
			ssidList[i] = new char[10];
		}
	};
	char * doathing(void)
	{
		ssidList[0] = "Hello World";
		return ssidList[0];
	};
private:
	char * ssidList [NUMPAIRS];
	float values [NUMPAIRS];
};

void setup() 
{
  
  Serial.begin(115200);
  
  // Before initializing the IMU, there are a few settings
  // we may need to adjust. Use the settings struct to set
  // the device's communication mode and addresses:
  imu.settings.device.commInterface = IMU_MODE_I2C;
  imu.settings.device.mAddress = LSM9DS1_M;
  imu.settings.device.agAddress = LSM9DS1_AG;
  // The above lines will only take effect AFTER calling
  // imu.begin(), which verifies communication with the IMU
  // and turns it on.
  pinMode( LSM9DS1_VDDIO_CTRL, OUTPUT );
  digitalWrite( LSM9DS1_VDDIO_CTRL, 0 );
  
  pinMode( LSM9DS1_VDD_CTRL, OUTPUT );
  digitalWrite( LSM9DS1_VDD_CTRL, 0 );
  
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
    while (1)
      ;
  }
}

void loop()
{

  if ( imu.magAvailable() )
  {
    // To read from the magnetometer, first call the
    // readMag() function. When it exits, it'll update the
    // mx, my, and mz variables with the most current data.
    imu.readMag();
  }
  
  if ((lastPrint + PRINT_SPEED) < millis())
  {
	  float magx = imu.calcMag(imu.mx);
	  float magy = imu.calcMag(imu.my);
	  magx += 1;
	  magy += 1;
	  if(magx < 0) magx = 0;
	  if(magx > 2) magx = 2;
	  if(magy < 0) magy = 0;
	  if(magy > 2) magy = 2;
	  
	  //Want to print "----------|---------- ----------|----------"
	  //Want to print "0---------|---------20 22---------|----------"
	  char tempStr[] = "----------|---------- ----------|----------";
	  tempStr[43] = 00;
	  tempStr[(int)(0 + 20 * magx / 2)] = '*';
	  tempStr[(int)(22 + 20 * magy / 2)] = '*';
	  Serial.println(tempStr);
    lastPrint = millis(); // Update lastPrint time
  }
  if ((lastScan + SCAN_SPEED) < millis())
  {
    int n = WiFi.scanNetworks();
    Serial.print("scan done: ");
    if (n == 0)
	{
        Serial.println("no networks found");
	}
	else
	{
		Serial.print(n);
		Serial.println(" networks found");
		//for (int i = 0; i < n; ++i) {
		//	// Print SSID and RSSI for each network found
		//	Serial.print(i + 1);
		//	Serial.print(": ");
		//	Serial.print(WiFi.SSID(i));
		//	Serial.print(" (");
		//	Serial.print(WiFi.RSSI(i));
		//	Serial.print(")");
		//	Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
		//	delay(10);
		//}
	}	  
    lastScan = millis(); // Update lastScan time
  }
}
