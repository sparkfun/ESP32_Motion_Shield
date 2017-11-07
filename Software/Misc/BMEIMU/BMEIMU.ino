/******************************************************************************
CSV_Output.ino
BME280 Arduino and Teensy example
Marshall Taylor @ SparkFun Electronics
May 20, 2015
https://github.com/sparkfun/SparkFun_BME280_Arduino_Library

This sketch configures two BME280 to produce comma separated values for use
in generating spreadsheet graphs.
in 10 being used for chip select.

Resources:
Uses Wire.h for I2C operation
Uses SPI.h for SPI operation

Development environment specifics:
Arduino IDE 1.6.4
Teensy loader 1.23

This code is released under the [MIT License](http://opensource.org/licenses/MIT).
Please review the LICENSE.md file included with this example. If you have any questions 
or concerns with licensing, please contact techsupport@sparkfun.com.
Distributed as-is; no warranty is given.
******************************************************************************/

#include <stdint.h>
#include "SparkFunBME280.h"

#include "Wire.h"
#include "SPI.h"

#include <SparkFunLSM9DS1.h>

LSM9DS1 imu;
#define LSM9DS1_M	0x1E // Would be 0x1C if SDO_M is LOW
#define LSM9DS1_AG	0x6B // Would be 0x6A if SDO_AG is LOW
#define LSM9DS1_VDD_CTRL 27
#define LSM9DS1_VDDIO_CTRL 34
#define PRINT_SPEED 250 // 250 ms between prints
static unsigned long lastPrint = 0; // Keep track of print time
#define DECLINATION -8.58 // Declination (degrees) in Boulder, CO.

//Internal variables
float roll;
float pitch;
float heading;
char csvBuffer[300];

//Global sensor object
BME280 mySensor;

unsigned int sampleNumber = 0; //For counting number of CSV rows

void setup()
{
	Serial.begin(115200);

	pinMode( LSM9DS1_VDDIO_CTRL, OUTPUT );
  digitalWrite( LSM9DS1_VDDIO_CTRL, 1 );
  
  pinMode( LSM9DS1_VDD_CTRL, OUTPUT );
  digitalWrite( LSM9DS1_VDD_CTRL, 1 );
  delay(100);
  digitalWrite( LSM9DS1_VDDIO_CTRL, 0 );
  digitalWrite( LSM9DS1_VDD_CTRL, 0 );
  
  delay(1000);
  Serial.println("Starting Sketch");

  //***Driver settings********************************//
	//commInterface can be I2C_MODE or SPI_MODE
	//specify chipSelectPin using arduino pin names
	//specify I2C address.  Can be 0x77(default) or 0x76
	
	//For I2C, enable the following and disable the SPI section
	mySensor.settings.commInterface = I2C_MODE;
	mySensor.settings.I2CAddress = 0x77;
	//***Operation settings*****************************//
	mySensor.settings.runMode = 3; //  3, Normal mode
	mySensor.settings.tStandby = 0; //  0, 0.5ms
	mySensor.settings.filter = 0; //  0, filter off
	mySensor.settings.tempOverSample = 1;
    mySensor.settings.pressOverSample = 1;
	mySensor.settings.humidOverSample = 1;
	
  imu.settings.device.commInterface = IMU_MODE_I2C;
  imu.settings.device.mAddress = LSM9DS1_M;
  imu.settings.device.agAddress = LSM9DS1_AG;
  // The above lines will only take effect AFTER calling
  // imu.begin(), which verifies communication with the IMU
  // and turns it on.
  imu.settings.mag.scale = 2;
  

	Serial.print("Starting BME280... result of .begin(): 0x");
	delay(10);  //Make sure sensor had enough time to turn on. BME280 requires 2ms to start up.
	//Calling .begin() causes the settings to be loaded
	Serial.println(mySensor.begin(), HEX);

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
	//Print each row in the loop
	//Start with temperature, as that data is needed for accurate compensation.
	//Reading the temperature updates the compensators of the other functions
	//in the background.
	Serial.print(sampleNumber);
	Serial.print(",");
	Serial.print(mySensor.readTempC(), 2);
	Serial.print(",");
	Serial.print(mySensor.readTempF(), 3);
	Serial.print(",");
	Serial.print(mySensor.readFloatPressure(), 0);
	Serial.print(",");
	Serial.print(mySensor.readFloatAltitudeMeters(), 3);
	Serial.print(",");
	Serial.print(mySensor.readFloatAltitudeFeet(), 3);
	Serial.print(",");
	Serial.print(mySensor.readFloatHumidity(), 0);
	Serial.println();
	
	sampleNumber++;

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
	//sprintf(csvBuffer, "%4.3f", heading);
	Serial.print(csvBuffer);
    Serial.println();
	
    Serial.println();
    
    lastPrint = millis(); // Update lastPrint time
	
  }

}

void printGyro()
{
  Serial.print("G: ");
  Serial.print(imu.calcGyro(imu.gx), 2);
  Serial.print(", ");
  Serial.print(imu.calcGyro(imu.gy), 2);
  Serial.print(", ");
  Serial.print(imu.calcGyro(imu.gz), 2);
  Serial.println(" deg/s");

}

void printAccel()
{  
  Serial.print("A: ");
  Serial.print(imu.calcAccel(imu.ax), 2);
  Serial.print(", ");
  Serial.print(imu.calcAccel(imu.ay), 2);
  Serial.print(", ");
  Serial.print(imu.calcAccel(imu.az), 2);
  Serial.println(" g");

}

void printMag()
{  
  Serial.print("M: ");
  Serial.print(imu.calcMag(imu.mx), 2);
  Serial.print(", ");
  Serial.print(imu.calcMag(imu.my), 2);
  Serial.print(", ");
  Serial.print(imu.calcMag(imu.mz), 2);
  Serial.println(" gauss");
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
  
  //Serial.print("Pitch, Roll: ");
  //Serial.print(pitch, 2);
  //Serial.print(", ");
  //Serial.println(roll, 2);
  //Serial.print("Heading: "); Serial.println(heading, 2);
}