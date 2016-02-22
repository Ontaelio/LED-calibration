#include <Wire.h>

uint8_t rgb[6];
uint16_t r, g, b;

void setup()
{
  Wire.begin(12);                // i2c address 12
  Wire.onRequest(giveData);      // register event
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
}

void loop()
{
  delay(100);
}

void giveData()
{
  r = analogRead(A0);
  g = analogRead(A1);
  b = analogRead(A2);
  
  rgb[0] = r>>8; // upper 2 bits
  rgb[1] = r&0xFF; // lower 8 bits
  
  rgb[2] = g>>8;
  rgb[3] = g&0xFF;
  
  rgb[4] = b>>8;
  rgb[5] = b&0xFF;
  
  Wire.write(rgb, 6); // send the array of bytes 
  
}
