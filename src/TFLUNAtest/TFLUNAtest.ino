#include <EVN.h>
#include <Wire.h>

#define TFLUNA_ADDR 0x10  // Default I2C address
EVNAlpha board;


int tfVal(byte port = 1) {
  board.setPort(port);
  int distance = 0;

  Wire.beginTransmission(0x10);
  Wire.write(0x00);             // Command to read distance (register 0x00)
  Wire.endTransmission(false);  // Send repeated start

  Wire.requestFrom(0x10, 3);  // Request 3 bytes: distance LSB, MSB, and checksum

  if (Wire.available() == 3) {
    byte lsb = Wire.read();
    byte msb = Wire.read();
    byte checksum = Wire.read();

    distance = (msb << 8) | lsb;

    //Serial.print("Distance: ");
    //Serial.print(distance);
    //Serial.println(" cm");
  }

  return distance; // 0 means error
  
}

void setup() {
  Serial.begin(115200);
  board.begin();
  
  //Wire.begin();
}

void loop() {
  Serial.println(tfVal(1));
  //delay(100);  // Small delay between reads
}
