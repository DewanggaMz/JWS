#include "prayer_schedule.h"

#include <PrayerTimes.h>
#include "services/prayer_times/prayer_times_service.h"
#include "date_and_time.h"

void getPrayerTimes(
) {
  JsonDocument configDocument;
  if (!loadPrayerTimesConfig(configDocument)) {
    Serial.println("ERROR: Gagal membaca konfigurasi prayerTimes");
    return;
  }

  String message;
  PrayerTimesConfig config;
  if (!parsePrayerTimesConfig(configDocument.as<JsonVariantConst>(), config, message)) {
    Serial.print("ERROR: ");
    Serial.println(message);
    return;
  }

  PrayerTimes pt(config.latitude, config.longitude, config.timezoneOffsetMinutes);

  if (!pt.isInitialized()) {
    Serial.print("ERROR: Invalid coordinates for ");
    return;
  }

  if (pt.isHighLatitude()) {
    Serial.println("  * Lokasi high-latitude terdeteksi");
  }

  applyPrayerTimesConfig(pt, config);


  Date dateNow = dayNow();
  Serial.printf("Prayer times for %d %d, %d\n", dateNow.day, dateNow.month, dateNow.year);

  PrayerTimesResult result = pt.calculate(dateNow.day, dateNow.month, dateNow.year);

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
