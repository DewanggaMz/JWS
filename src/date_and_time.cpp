#include "date_and_time.h"

#include "config.h"
#include "utils/utils.h"

RTC_DS3231 rtc;


const char* namaHari[] = {
  "Minggu",
  "Senin",
  "Selasa",
  "Rabu",
  "Kamis",
  "Jumat",
  "Sabtu"
};

namespace{
  void dateTimeCalibration ( uint8_t day, uint8_t month, uint16_t year ,uint8_t hour, uint8_t minute, uint8_t second){
    rtc.adjust(DateTime(year, month, day, hour, minute, second));
    delay(1000);
  }
}

void initTime () {
  Wire.begin(PIN_SDA, PIN_SCL);

  if (!rtc.begin()) {
    Serial.println("RTC DS3231 tidak ditemukan!");
    while (1);
  }

  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

Time timeNow () {
  DateTime now = rtc.now();

  Time time;
  time.hour = now.hour();
  time.minute = now.minute();
  time.second = now.second();

  return time;
}

Date dayNow () {
  DateTime now = rtc.now();

  Date date;
  date.dayName = namaHari[now.dayOfTheWeek()];
  date.day = now.day();
  date.month = now.month();
  date.year = now.year();
  return date;
}

bool updateDateTimeAdjustment(JsonVariantConst payload, String &message) {
  uint8_t day = payload["day"].as<uint8_t>();
  uint8_t month = payload["month"].as<uint8_t>();
  uint16_t year = payload["year"].as<uint16_t>();
  uint8_t hour = payload["hour"].as<uint8_t>();
  uint8_t minute = payload["minute"].as<uint8_t>();
  uint8_t second = payload["second"].as<uint8_t>();

  dateTimeCalibration(day, month, year, hour, minute, second);
  message = "Waktu berhasil diatur ke " + formatDate(day, month, year) + " " + formatTime(hour, minute, second);
  return true;
}