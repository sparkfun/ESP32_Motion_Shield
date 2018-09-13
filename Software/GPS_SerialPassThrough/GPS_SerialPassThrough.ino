#include <Arduino.h>
HardwareSerial GPS(2);  // UART1/Serial1 pins 16,17

void setup()
{
  Serial.begin(115200);
  GPS.begin(9600);
  delay(1000);
  Serial.println("Sketch Started.");
}

void loop() {
  //Pass usb data to the gps
  if (Serial.available())
  {
    GPS.write(Serial.read());
  }
  //Pass gps data to the usb
  if (GPS.available())
  {
    Serial.write(GPS.read());
  }
}
