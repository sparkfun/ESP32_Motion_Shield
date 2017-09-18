#include <Arduino.h>

#define CD_PIN 38

void setup(){
    Serial.begin(115200);
	pinMode(CD_PIN, INPUT_PULLUP);
	pinMode(37, OUTPUT);


}

int toggler = 0;

void loop(){
	delay(1000);
	Serial.print("CD Inserted? -- ");
	if(digitalRead(CD_PIN) == 1)
	{
		Serial.println("Yes.");
	}
	else
	{
		Serial.println("No.");
	}
	digitalWrite(37, toggler);
	toggler ^= 0x01;
}
