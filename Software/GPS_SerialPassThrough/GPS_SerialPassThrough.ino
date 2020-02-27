#include <Arduino.h>
HardwareSerial GPSUART(2);  // Set up GPS UART on pins 16 & 17

void setup()
{
  Serial.begin(115200);
  GPSUART.begin(9600);
  delay(1000);
  Serial.println("Sketch Started.");
}

void loop() {
  //Pass usb data to the gps
  if (Serial.available())
  {
    GPSUART.write(Serial.read());
  }
  //Pass gps data to the usb
  if (GPSUART.available())
  {
    Serial.write(GPSUART.read());
  }
}
