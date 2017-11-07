#include <Arduino.h>

HardwareSerial Serial1(2);  // UART1/Serial1 pins 16,17

#define AUX_LED_PIN 13

void setup(){
	pinMode(AUX_LED_PIN, OUTPUT);

    Serial.begin(115200);
	Serial1.begin(9600);
	
	delay(1000);
	Serial.println("Starting Sketch.");
	Serial.println("Loopback GPS pins, then enter serial characters.");
	Serial.println("Test JST by installing GPS receiver.");
	digitalWrite(AUX_LED_PIN, 1);
	
}
int toggler = 0;

void loop(){
	char c;
	if(Serial.available())
	{
		c = Serial.read();
		Serial1.print(c);
		digitalWrite(AUX_LED_PIN, toggler);
		toggler ^= 0x01;
	}
	if(Serial1.available())
	{
		c = Serial1.read();
		Serial.print(c);
	}
	delay(10);
}