#include <Arduino.h>

void setup(){
    Serial.begin(115200);

	pinMode(34, INPUT);
	pinMode(35, INPUT);
	pinMode(36, INPUT);
	pinMode(37, INPUT);
	pinMode(38, INPUT);
	pinMode(39, INPUT);


}

int toggler = 0;

void loop(){
	delay(1000);
	Serial.print(digitalRead(34));
	Serial.print(", ");
	Serial.print(digitalRead(35));
	Serial.print(", ");
	Serial.print(digitalRead(36));
	Serial.print(", ");
	Serial.print(digitalRead(37));
	Serial.print(", ");
	Serial.print(digitalRead(38));
	Serial.print(", ");
	Serial.print(digitalRead(39));
	Serial.println();
}
