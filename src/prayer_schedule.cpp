#include "prayer_schedule.h"

#include <PrayerTimes.h>
#include "config.h"

static const int DAY   = 20;
static const int MONTH = 6;
static const int YEAR  = 2026;

const int LATITUDE = -8.245230;
const int LONGITUDE = 112.600482;
const int GMT_OFFSET = 7 * 60;


void getPrayerTimes(
) {
  
  PrayerTimes pt(LATITUDE, LONGITUDE, GMT_OFFSET);

  if (!pt.isInitialized()) {
    Serial.print("ERROR: Invalid coordinates for ");
    return;
  }
  
  if (pt.isHighLatitude()) {
    pt.setHighLatitudeRule(NONE);
  }
  
  pt.setCustomMethod(-20.0, -18.0);
  pt.setAsrMethod(SHAFII);
  pt.setCalculationMethod(CalculationMethods::INDONESIA);
  pt.setImsakOffset(10);
  pt.setDuhaAngle(4);
  pt.setAdjustments(2, 0, 3, 2, 5, 3);

  PrayerTimesResult result = pt.calculate(DAY, MONTH, YEAR);

  if (!result.valid) {
    Serial.print("ERROR: Calculation failed for ");
    return;
  }
  Serial.println();
  
  int hour, minute;
  
  // Imsak
  pt.minutesToTime(result.imsak, hour, minute);
  Serial.print("  Imsak                 : ");
  Serial.println(pt.formatTime24(hour, minute));
  
  // Fajr
  pt.minutesToTime(result.fajr, hour, minute);
  Serial.print("  Fajr                  : ");
  Serial.println(pt.formatTime24(hour, minute));
  
  // Sunrise
  pt.minutesToTime(result.sunrise, hour, minute);
  Serial.print("  Terbit                : ");
  Serial.println(pt.formatTime24(hour, minute));
  
  // Duha
  pt.minutesToTime(result.duha, hour, minute);
  Serial.print("  Duha                  : ");
  Serial.println(pt.formatTime24(hour, minute));
  
  // Dhuhr
  pt.minutesToTime(result.dhuhr, hour, minute);
  Serial.print("  Dhuhr                 : ");
  Serial.println(pt.formatTime24(hour, minute));

  //ashar
  pt.minutesToTime(result.asr, hour, minute);
  Serial.print("  Ashar                 : ");
  Serial.println(pt.formatTime24(hour, minute));
  
  // Maghrib
  pt.minutesToTime(result.maghrib, hour, minute);
  Serial.print("  Maghrib               : ");
  Serial.println(pt.formatTime24(hour, minute));


  pt.minutesToTime(result.isha, hour, minute);
  Serial.print("  Isha                  : ");
  Serial.println(pt.formatTime24(hour, minute));


  
  if (pt.isHighLatitude()) {
    Serial.println();
    Serial.println("  * High-latitude adjustments applied");
  }
}