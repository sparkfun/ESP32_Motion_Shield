/*****************************************************************
LSM9DS1_CSV.ino
Collecting IMU data as CSV for graphing

Original Creation: August 13, 2015 by Jim Lindblom
from the LSM9DS1_Basic_I2C.ino library example.
https://github.com/sparkfun/LSM9DS1_Breakout

Hardware setup: This library is intended to be used with a 
ESP32 Motion shield connected directly to the ESP32 Thing.

Development environment specifics:
  IDE: Arduino 1.8.2
  Hardware Platform: ESP32 Arduion Board

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

void setup() 
{
  Serial.begin(115200);
  
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
    Serial.println("Double-check connections.");
    while (1)
      ;
  }
  
}

void loop()
{
  // Update the sensor values whenever new data is available
  if ( imu.gyroAvailable() )
  {
    // To read from the gyroscope,  first call the
    // readGyro() function. When it exits, it'll update the
    // gx, gy, and gz variables with the most current data.
    imu.readGyro();
  }
  if ( imu.accelAvailable() )
  {
    // To read from the accelerometer, first call the
    // readAccel() function. When it exits, it'll update the
    // ax, ay, and az variables with the most current data.
    imu.readAccel();
  }
  if ( imu.magAvailable() )
  {
    // To read from the magnetometer, first call the
    // readMag() function. When it exits, it'll update the
    // mx, my, and mz variables with the most current data.
    imu.readMag();
  }
  
  if ((lastPrint + PRINT_SPEED) < millis())
  {
    // Print the collected data as CSV
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
	Serial.print(csvBuffer);
    Serial.println();
    
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