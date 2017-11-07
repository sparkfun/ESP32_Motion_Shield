#include <Arduino.h>
HardwareSerial Serial1(2);  // UART1/Serial1 pins 16,17

void setup()
{
  Serial.begin(115200);
  Serial1.begin(9600);
  delay(1000);
  Serial.println("Sketch Started.");
}

void loop() {
  //Pass usb data to the gps
  if (Serial.available())
  {
    Serial1.write(Serial.read());
  }
  //Pass gps data to the usb
  if (Serial1.available())
  {
    Serial.write(Serial1.read());
  }
}