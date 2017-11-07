#include <Arduino.h>

#define AUX_LED_PIN 13
#define GPS_TX_PIN 16
#define GPS_RX_PIN 17
#define SD_SCK_PIN 18
#define SD_DO_PIN 19
#define SD_DI_PIN 23
#define SD_CS_PIN 33
#define SD_CD_PIN 38
#define IMU_SDA_PIN 21
#define IMU_SCL_PIN 22

void setup(){
    Serial.begin(115200);

	pinMode(AUX_LED_PIN, OUTPUT);
	digitalWrite(AUX_LED_PIN, 1);

	pinMode(SD_CD_PIN, INPUT);

	Serial.println("Sketch Started.");
	Serial.println("D13 LED should be blinking.");

}

void loop(){
	delay(1000);
	Serial.print("CD Inserted? -- ");
	if(digitalRead(SD_CD_PIN) == 1)
	{
		Serial.println("Yes.");
	}
	else
	{
		Serial.println("No.");
	}

	digitalWrite(AUX_LED_PIN, digitalRead(AUX_LED_PIN)^0x01);
}
