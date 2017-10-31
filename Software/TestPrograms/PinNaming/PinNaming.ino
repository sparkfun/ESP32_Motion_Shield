#include <Arduino.h>

#define AUX_LED_PIN 13
#define SD_CS_PIN 38
#define IMU_V_CTRL 27

int state = 1;

void setup(){
    Serial.begin(115200);

	pinMode(AUX_LED_PIN, OUTPUT);
	digitalWrite(AUX_LED_PIN, state^0x01);
	pinMode(IMU_V_CTRL, OUTPUT);
	digitalWrite(IMU_V_CTRL, state);
	Serial.println("Sketch Started, enter 't' to toggle");

}

void loop(){
	if(Serial.available())
	{
		if(Serial.read() == 't')
		{
			state ^= 0x01;
			digitalWrite(AUX_LED_PIN, state^0x01);
			digitalWrite(IMU_V_CTRL, state);
		}
	}
}
