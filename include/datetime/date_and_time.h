#pragma once

#include <Arduino.h>
#include <RTClib.h>
#include <Wire.h>
#include <ArduinoJson.h>

extern RTC_DS3231 rtc;

struct Date {
  const char* dayName;
  uint8_t day;
  uint8_t month;
  uint16_t year;
};

struct Time {
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
};

struct DateTimeState {
  Date date;
  Time time;
};

bool initTime();
bool dateTimeNow(DateTimeState &state);
Time timeNow ();
Date dayNow ();
bool updateDateTimeAdjustment(JsonVariantConst payload, String &message);
long gregorianToJDN(int year, int month, int day);
