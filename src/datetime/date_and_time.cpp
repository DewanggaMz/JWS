#include "datetime/date_and_time.h"

#include "config.h"
#include "utils/utils.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

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
  SemaphoreHandle_t rtcMutex = nullptr;

  class RtcGuard {
    public:
      RtcGuard()
        : locked(
            rtcMutex != nullptr &&
            xSemaphoreTake(
              rtcMutex,
              pdMS_TO_TICKS(1000)
            ) == pdTRUE
          )
      {
      }

      ~RtcGuard()
      {
        if (locked) {
          xSemaphoreGive(rtcMutex);
        }
      }

      explicit operator bool() const
      {
        return locked;
      }

    private:
      bool locked;
  };

  void dateTimeCalibration(uint8_t day, uint8_t month, uint16_t year, uint8_t hour, uint8_t minute, uint8_t second) {
    rtc.adjust(DateTime(year, month, day, hour, minute, second));
  }

  bool isLeapYear(uint16_t year) {
    return (year % 4 == 0 && year % 100 != 0) ||
           (year % 400 == 0);
  }

  uint8_t daysInMonth(uint8_t month, uint16_t year) {
    static const uint8_t days[] = {
      31, 28, 31, 30, 31, 30,
      31, 31, 30, 31, 30, 31
    };
    if (month == 2 && isLeapYear(year)) {
      return 29;
    }
    return days[month - 1];
  }
}

bool initTime() {
  if (rtcMutex == nullptr) {
    rtcMutex = xSemaphoreCreateMutex();
  }
  if (rtcMutex == nullptr) {
    return false;
  }

  Wire.begin(PIN_SDA, PIN_SCL);

  if (!rtc.begin()) {
    Serial.println("RTC DS3231 tidak ditemukan!");
    return false;
  }

  return true;
}

bool dateTimeNow(DateTimeState &state) {
  RtcGuard guard;
  if (!guard) {
    return false;
  }

  DateTime now = rtc.now();

  state.time.hour = now.hour();
  state.time.minute = now.minute();
  state.time.second = now.second();
  state.date.dayName = namaHari[now.dayOfTheWeek()];
  state.date.day = now.day();
  state.date.month = now.month();
  state.date.year = now.year();
  return true;
}

Time timeNow() {
  DateTimeState state;
  dateTimeNow(state);
  return state.time;
}

Date dayNow() {
  DateTimeState state;
  dateTimeNow(state);
  return state.date;
}

bool updateDateTimeAdjustment(JsonVariantConst payload, String &message) {
  if (!payload.is<JsonObjectConst>()) {
    message = "Payload waktu harus berupa object";
    return false;
  }

  DateTimeState current;
  dateTimeNow(current);
  const bool hasDate =
    !payload["day"].isUnbound() ||
    !payload["month"].isUnbound() ||
    !payload["year"].isUnbound();
  const bool hasTime =
    !payload["hour"].isUnbound() ||
    !payload["minute"].isUnbound() ||
    !payload["second"].isUnbound();

  if (hasDate &&
      (!payload["day"].is<int>() ||
       !payload["month"].is<int>() ||
       !payload["year"].is<int>())) {
    message = "day, month, dan year harus dikirim lengkap sebagai angka";
    return false;
  }
  if (hasTime &&
      (!payload["hour"].is<int>() ||
       !payload["minute"].is<int>() ||
       !payload["second"].is<int>())) {
    message = "hour, minute, dan second harus dikirim lengkap sebagai angka";
    return false;
  }

  const int day = hasDate
                    ? payload["day"].as<int>()
                    : current.date.day;
  const int month = hasDate
                      ? payload["month"].as<int>()
                      : current.date.month;
  const int year = hasDate
                     ? payload["year"].as<int>()
                     : current.date.year;
  const int hour = hasTime
                     ? payload["hour"].as<int>()
                     : current.time.hour;
  const int minute = hasTime
                       ? payload["minute"].as<int>()
                       : current.time.minute;
  const int second = hasTime
                       ? payload["second"].as<int>()
                       : current.time.second;

  if (year < 2000 || year > 2099 ||
      month < 1 || month > 12 ||
      day < 1 || day > daysInMonth(month, year) ||
      hour < 0 || hour > 23 ||
      minute < 0 || minute > 59 ||
      second < 0 || second > 59) {
    message = "Tanggal atau waktu tidak valid";
    return false;
  }

  RtcGuard guard;
  if (!guard) {
    message = "RTC sedang digunakan";
    return false;
  }
  dateTimeCalibration(day, month, year, hour, minute, second);
  message = "Waktu berhasil diatur ke " + formatDate(day, month, year) + " " + formatTime(hour, minute, second);
  return true;
}

long gregorianToJDN(int year, int month, int day) {
  int a = (14 - month) / 12;
  int y = year + 4800 - a;
  int m = month + 12 * a - 3;

  long jdn = day
             + (153L * m + 2) / 5
             + 365L * y
             + y / 4
             - y / 100
             + y / 400
             - 32045;

  return jdn;
}
