/******************************************************************************
BMEIMU.ino
ESP32 Motion Shield & BME280 Example
Marshall Taylor @ SparkFun Electronics
May 20, 2015
https://github.com/sparkfun/SparkFun_BME280_Arduino_Library

This sketch configures the LSM9DS1 on the ESP32 Motion Shield along with a BME280 to produce comma separated values for use
in generating spreadsheet graphs.


Resources:
Uses Wire.h for I2C operation

Development environment specifics:
Arduino IDE 1.6.4

This code is released under the [MIT License](http://opensource.org/licenses/MIT).
Please review the LICENSE.md file included with this example. If you have any questions 
or concerns with licensing, please contact techsupport@sparkfun.com.
Distributed as-is; no warranty is given.
******************************************************************************/

#include <stdint.h>
#include "SparkFunBME280.h"
#include <SparkFunLSM9DS1.h>

#include "Wire.h"
#include "SPI.h"


BME280 mySensor; // Create BME280 sensor object
LSM9DS1 imu;  // Create IMU object

#define PRINT_SPEED 250 // 250 ms between prints
static unsigned long lastPrint = 0; // Keep track of print time
#define DECLINATION -8.58 // Declination (degrees) in Boulder, CO.

//Internal variables
float roll;
float pitch;
float heading;
char csvBuffer[300];

//Function definitions
void printGyro();
void printAccel();
void printMag();
void printAttitude(float ax, float ay, float az, float mx, float my, float mz);


unsigned int sampleNumber = 0; //For counting number of CSV rows

void setup()
{
	Serial.begin(115200);
  Serial.println("Starting Sketch");
  Wire.begin();

  if (mySensor.beginI2C() == false) //Begin communication over I2C
  {
    Serial.println("The sensor did not respond. Please check wiring.");
    while(1); //Freeze
  }
  delay(10);
  mySensor.setFilter(1); //0 to 4 is valid. Filter coefficient. See 3.4.4
  mySensor.setStandbyTime(0); //0 to 7 valid. Time between readings. See table 27.

  mySensor.setTempOverSample(1); //0 to 16 are valid. 0 disables temp sensing. See table 24.
  mySensor.setPressureOverSample(1); //0 to 16 are valid. 0 disables pressure sensing. See table 23.
  mySensor.setHumidityOverSample(1); //0 to 16 are valid. 0 disables humidity sensing. See table 19.
  
  mySensor.setMode(MODE_NORMAL); //MODE_SLEEP, MODE_FORCED, MODE_NORMAL is valid. See 3.3
	
  if (imu.begin() == false) // with no arguments, this uses default addresses (AG:0x6B, M:0x1E) and i2c port (Wire).
  {
    Serial.println("Failed to communicate with LSM9DS1.");
    Serial.println("Double-check wiring.");
    Serial.println("Default settings in this sketch will " \
                   "work for an out of the box LSM9DS1 " \
                   "Breakout, but may need to be modified " \
                   "if the board jumpers are.");
    while (1);
  }
	delay(100);

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
