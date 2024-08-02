#include <Wire.h>
#include <SPI.h>  // not used here, but needed to prevent a RTClib compile error
#include "RTClib.h"

RTC_DS1307 RTC;     // Setup an instance of DS1307 naming it RTC

void setup () {
  Serial.begin(57600); // Set serial port speed
  Wire.begin(); // Start the I2C
  RTC.begin();  // Init RTC
  
  // Call function to set custom date and time
  setDateTime(2024, 7, 29, 12, 2, 0);  // Set to July 29, 2024, 15:30:00
  
  Serial.println("Time and date set");
}

void loop () {
  DateTime now = RTC.now();

  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(' ');
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();

  delay(1000);
}

//Function to set custom date and time
void setDateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second) {
  DateTime customDateTime = DateTime(year, month, day, hour, minute, second);
  RTC.adjust(customDateTime);
}