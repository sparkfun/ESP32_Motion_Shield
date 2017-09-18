#include <Arduino.h>
#include "fileHandlers.h"
#include <SparkFunLSM9DS1.h>
#include "SparkFunBME280.h"
#include "Wire.h"
#include "SPI.h"

//Global sensor object
BME280 mySensor;

unsigned int sampleNumber = 0; //For counting number of CSV rows

HardwareSerial Serial1(2);  // UART1/Serial1 pins 16,17

LogDumper GPSLogFile;
LogDumper IMULogFile;
LogDumper BMELogFile;

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

void setup(){
    Serial.begin(115200);
	Serial1.begin(9600);
	//IMU

	mySensor.settings.commInterface = SPI_MODE;
	mySensor.settings.chipSelectPin = 14;
	mySensor.settings.runMode = 3; //  3, Normal mode
	mySensor.settings.tStandby = 0; //  0, 0.5ms
	mySensor.settings.filter = 0; //  0, filter off
	mySensor.settings.tempOverSample = 1;
    mySensor.settings.pressOverSample = 1;
	mySensor.settings.humidOverSample = 1;
	
	Serial.begin(115200);
	Serial.print("Program Started\n");
	Serial.print("Starting BME280... result of .begin(): 0x");
	delay(10);  //Make sure sensor had enough time to turn on. BME280 requires 2ms to start up.
	//Calling .begin() causes the settings to be loaded
	Serial.println(mySensor.begin(), HEX);

	imu.settings.device.commInterface = IMU_MODE_I2C;
	imu.settings.device.mAddress = LSM9DS1_M;
	imu.settings.device.agAddress = LSM9DS1_AG;
	pinMode( LSM9DS1_VDDIO_CTRL, OUTPUT );
	digitalWrite( LSM9DS1_VDDIO_CTRL, 1 );
	
	pinMode( LSM9DS1_VDD_CTRL, OUTPUT );
	digitalWrite( LSM9DS1_VDD_CTRL, 1 );
	delay(100);
	digitalWrite( LSM9DS1_VDDIO_CTRL, 0 );
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
  
    if(!SD.begin(33, SPI, 1000000, "/sd")){
        Serial.println("Card Mount Failed");
        return;
    }
    uint8_t cardType = SD.cardType();
	
    if(cardType == CARD_NONE){
        Serial.println("No SD card attached");
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

	GPSLogFile.setDevice(&SD);
	GPSLogFile.setPath("multiLogPath3");
	GPSLogFile.setFileNameStub("GPSLog");
	GPSLogFile.startLog();

	IMULogFile.setDevice(&SD);
	IMULogFile.setPath("multiLogPath3");
	IMULogFile.setFileNameStub("IMULog");
	IMULogFile.startLog();

	BMELogFile.setDevice(&SD);
	BMELogFile.setPath("multiLogPath3");
	BMELogFile.setFileNameStub("BMELog");
	BMELogFile.startLog();
	
	BMELogFile.print("\n\n");
	BMELogFile.print("Sample,");
	BMELogFile.print("T(deg C),");
	BMELogFile.print("T(deg F),");
	BMELogFile.print("P(Pa),");
	BMELogFile.print("Alt(m),");
	BMELogFile.print("Alt(ft),");
	BMELogFile.print("%RH");
	BMELogFile.print(0x0D);
	BMELogFile.print(0x0A);

}

int iGlobal = 0;
int jGlobal = 0;
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
					parseBuffer[parseBufferPtr] = 0x0D;
					parseBufferPtr++;
					parseBuffer[parseBufferPtr] = 0x0A;
					parseBufferPtr++;
					parseBuffer[parseBufferPtr] = 0;
					parseBufferPtr++;
					//Pick out good messsage
					if(( parseBuffer[1] == 'G')&&( parseBuffer[2] == 'P')&&( parseBuffer[3] == 'R')&&( parseBuffer[4] == 'M')&&( parseBuffer[5] == 'C'))
					{
						GPSLogFile.print(parseBuffer);
						//Serial.print(parseBuffer);
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
	if ( imu.gyroAvailable() ) imu.readGyro();
	if ( imu.accelAvailable() ) imu.readAccel();
	if ( imu.magAvailable() ) imu.readMag();
	if ((lastPrint + PRINT_SPEED) < millis())
	{
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
		//Serial.print(csvBuffer);
		//Serial.println();
		IMULogFile.print(csvBuffer);
		IMULogFile.print(0x0D);
		IMULogFile.print(0x0A);

		char temp[20];
		sprintf(temp, "%d", sampleNumber);
		BMELogFile.print(temp);
		BMELogFile.print(",");
		sprintf(temp, "%4.3f", mySensor.readTempC());
		BMELogFile.print(temp);
		BMELogFile.print(",");
		sprintf(temp, "%4.3f", mySensor.readTempF());
		BMELogFile.print(temp);
		BMELogFile.print(",");
		sprintf(temp, "%4.3f", mySensor.readFloatPressure());
		BMELogFile.print(temp);
		BMELogFile.print(",");
		sprintf(temp, "%4.3f", mySensor.readFloatAltitudeMeters());
		BMELogFile.print(temp);
		BMELogFile.print(",");
		sprintf(temp, "%4.3f", mySensor.readFloatAltitudeFeet());
		BMELogFile.print(temp);
		BMELogFile.print(",");
		sprintf(temp, "%4.3f", mySensor.readFloatHumidity());
		BMELogFile.print(temp);
		BMELogFile.print(0x0D);
		BMELogFile.print(0x0A);
		
		sampleNumber++;
	
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