#include "services/prayer_times/prayer_times_service.h"

#include "config.h"
#include "services/database/database_service.h"
#include "storage.h"

namespace {

bool isKnownCalculationMethod(const String &method) {
  return method == "ISNA" || method == "EGYPT" || method == "MAKKAH"  || method == "INDONESIA" || method == "CUSTOM";
}

// ==================================================================================================

bool isKnownAsrMethod(const String &method) {
  return method == "SHAFII" || method == "HANAFI";
}

// ==================================================================================================

bool isKnownHighLatitudeRule(const String &rule) {
  return rule == "NONE" || rule == "MIDDLE_OF_NIGHT" || rule == "ONE_SEVENTH" ||
         rule == "ANGLE_BASED";
}


// ==================================================================================================

void mergeJsonObject(JsonObject target, JsonObjectConst source) {
  for (JsonPairConst item : source) {
    const char *key = item.key().c_str();

    JsonVariant targetValue = target[key];

    if (targetValue.isNull()) {
      // Serial.printf("Key %s tidak ada di struktur database.json, dilewati\n", key);
      continue;
    }

    if (item.value().is<JsonObjectConst>()) {
      if (!targetValue.is<JsonObject>()) {
        Serial.printf("Key %s bukan object di database.json, dilewati\n", key);
        continue;
      }

      mergeJsonObject(targetValue.as<JsonObject>(), item.value().as<JsonObjectConst>());
      continue;
    }

    // Serial.printf("Mengganti nilai %s: %s\n", key, item.value().as<String>().c_str());

    targetValue.set(item.value());
  }
}

// ==================================================================================================

bool loadDefaultPrayerTimesConfig(JsonDocument &config) {
  JsonDocument defaultFile;
  if (readJsonFile(DEFAULT_PRAYER_TIMES_PATH, defaultFile)) {
    if (defaultFile["prayerTimesConfig"].is<JsonObject>()) {
      config.set(defaultFile["prayerTimesConfig"]);
      return true;
    }

    if (defaultFile.is<JsonObject>()) {
      config.set(defaultFile);
      return true;
    }
  }

  config.clear();
  JsonObject root = config.to<JsonObject>();
  setDefaultPrayerTimesConfig(root);
  return true;
}

// ==================================================================================================

void setCalculationMethod(PrayerTimes &prayerTimes, const PrayerTimesConfig &config) {
  if (config.calculationMethod == "ISNA") prayerTimes.setCalculationMethod(CalculationMethods::ISNA);
  else if (config.calculationMethod == "EGYPT") prayerTimes.setCalculationMethod(CalculationMethods::EGYPT);
  else if (config.calculationMethod == "MAKKAH") prayerTimes.setCalculationMethod(CalculationMethods::MAKKAH);
  else if (config.calculationMethod == "CUSTOM") {
    prayerTimes.setCustomMethod(
      config.fajrAngle,
      config.ishaAngle,
      config.ishaIsInterval,
      config.ishaMinutes
    );
  } else {
    prayerTimes.setCalculationMethod(CalculationMethods::INDONESIA);
  }
}

}

// ==================================================================================================

void setDefaultPrayerTimesConfig(JsonObject target) {
  JsonObject location = target["location"].to<JsonObject>();
  location["latitude"] = -8.245230;
  location["longitude"] = 112.600482;
  location["timezoneOffsetMinutes"] = 420;

  JsonObject calculation = target["calculation"].to<JsonObject>();
  calculation["method"] = "INDONESIA";
  calculation["fajrAngle"] = 20.0;
  calculation["ishaAngle"] = 18.0;
  calculation["ishaIsInterval"] = false;
  calculation["ishaMinutes"] = 0;
  calculation["asrMethod"] = "SHAFII";
  calculation["highLatitudeRule"] = "NONE";
  calculation["imsakOffsetMinutes"] = 10;
  calculation["duhaAngle"] = 4.0;

  JsonObject adjustments = calculation["adjustments"].to<JsonObject>();
  adjustments["fajr"] = 0;
  adjustments["sunrise"] = 0;
  adjustments["dhuhr"] = 0;
  adjustments["asr"] = 0;
  adjustments["maghrib"] = 0;
  adjustments["isha"] = 0;
}

// ==================================================================================================

bool ensurePrayerTimesConfig() {
  JsonDocument database;
  loadDatabase(database);

  if (database["prayerTimesConfig"].is<JsonObject>() && database["prayerTimesConfig"].size() > 0) {
    return true;
  }

  JsonDocument defaultConfig;
  loadDefaultPrayerTimesConfig(defaultConfig);
  database["prayerTimesConfig"].set(defaultConfig.as<JsonVariantConst>());

  return saveDatabase(database);
}

// ==================================================================================================

bool loadPrayerTimesConfig(JsonDocument &config) {
  JsonDocument database;
  loadDatabase(database);

  if (!database["prayerTimesConfig"].is<JsonObject>()) {
    if (!ensurePrayerTimesConfig()) {
      return false;
    }
    loadDatabase(database);
  }

  config.clear();
  config.set(database["prayerTimesConfig"]);
  return config.is<JsonObject>();
}


// ==================================================================================================

bool updatePrayerTimesConfig(JsonVariantConst payload, String &message) {
  if (!payload.is<JsonObjectConst>()) {
    message = "Body JSON harus berupa object";
    return false;
  }

  JsonVariantConst incoming = payload["prayerTimesConfig"].is<JsonObjectConst>()
                                ? payload["prayerTimesConfig"]
                                : payload;

  // print incoming
  String incomingJson;
  serializeJson(incoming, incomingJson);
  Serial.println(incomingJson);

  JsonDocument database;
  loadDatabase(database);

  if (!database["prayerTimesConfig"].is<JsonObject>()) {
    JsonDocument defaultConfig;
    loadDefaultPrayerTimesConfig(defaultConfig);
    database["prayerTimesConfig"].set(defaultConfig.as<JsonVariantConst>());
  }

  // print database
  // String databaseJson;
  // serializeJson(database, databaseJson);
  // Serial.print("database :  ");
  // Serial.println(databaseJson);

  // Serial.print("database prayer :  ");


  JsonObject prayerTimes = database["prayerTimesConfig"].as<JsonObject>();

  // print prayerTimes
  String prayerTimesJson;
  serializeJson(prayerTimes, prayerTimesJson);
  Serial.println("prayerTimes");
  Serial.println(prayerTimesJson);

  mergeJsonObject(prayerTimes, incoming.as<JsonObjectConst>());

  PrayerTimesConfig parsedConfig;
  if (!parsePrayerTimesConfig(prayerTimes, parsedConfig, message)) {
    return false;
  }

  if (!saveDatabase(database)) {
    message = "Gagal menyimpan konfigurasi prayerTimes ke database.json";
    return false;
  }

  message = "Konfigurasi prayerTimes berhasil diperbarui";
  return true;
}

// ==================================================================================================

bool parsePrayerTimesConfig(JsonVariantConst source, PrayerTimesConfig &config, String &message) {
  if (!source.is<JsonObjectConst>()) {
    message = "Konfigurasi prayerTimes tidak valid";
    return false;
  }

  JsonObjectConst location = source["location"];
  JsonObjectConst calculation = source["calculation"];
  JsonObjectConst adjustments = calculation["adjustments"];

  config.latitude = location["latitude"] | -8.245230;
  config.longitude = location["longitude"] | 112.600482;
  config.timezoneOffsetMinutes = location["timezoneOffsetMinutes"] | 420;
  config.calculationMethod = calculation["method"] | "INDONESIA";
  config.fajrAngle = calculation["fajrAngle"] | 20.0;
  config.ishaAngle = calculation["ishaAngle"] | 18.0;
  config.ishaIsInterval = calculation["ishaIsInterval"] | false;
  config.ishaMinutes = calculation["ishaMinutes"] | 0;
  config.asrMethod = calculation["asrMethod"] | "SHAFII";
  config.highLatitudeRule = calculation["highLatitudeRule"] | "NONE";
  config.imsakOffsetMinutes = calculation["imsakOffsetMinutes"] | 10;
  config.duhaAngle = calculation["duhaAngle"] | 4.0;
  config.adjustmentFajr = adjustments["fajr"] | 0;
  config.adjustmentSunrise = adjustments["sunrise"] | 0;
  config.adjustmentDhuhr = adjustments["dhuhr"] | 0;
  config.adjustmentAsr = adjustments["asr"] | 0;
  config.adjustmentMaghrib = adjustments["maghrib"] | 0;
  config.adjustmentIsha = adjustments["isha"] | 0;

  if (config.latitude < -90.0 || config.latitude > 90.0 ||
      config.longitude < -180.0 || config.longitude > 180.0) {
    message = "Koordinat prayerTimes tidak valid";
    return false;
  }

  if (!isKnownCalculationMethod(config.calculationMethod)) {
    message = "Metode kalkulasi prayerTimes tidak dikenal";
    return false;
  }

  if (!isKnownAsrMethod(config.asrMethod)) {
    message = "Metode asar prayerTimes tidak dikenal";
    return false;
  }

  if (!isKnownHighLatitudeRule(config.highLatitudeRule)) {
    message = "Aturan high latitude prayerTimes tidak dikenal";
    return false;
  }

  return true;
}

// ==================================================================================================

void applyPrayerTimesConfig(PrayerTimes &prayerTimes, const PrayerTimesConfig &config) {
  setCalculationMethod(prayerTimes, config);
  prayerTimes.setAsrMethod(config.asrMethod == "HANAFI" ? HANAFI : SHAFII);

  if (config.highLatitudeRule == "MIDDLE_OF_NIGHT") {
    prayerTimes.setHighLatitudeRule(MIDDLE_OF_NIGHT);
  } else if (config.highLatitudeRule == "ONE_SEVENTH") {
    prayerTimes.setHighLatitudeRule(ONE_SEVENTH);
  } else if (config.highLatitudeRule == "ANGLE_BASED") {
    prayerTimes.setHighLatitudeRule(ANGLE_BASED);
  } else {
    prayerTimes.setHighLatitudeRule(NONE);
  }

  prayerTimes.setImsakOffset(config.imsakOffsetMinutes);
  prayerTimes.setDuhaAngle(config.duhaAngle);
  prayerTimes.setAdjustments(
    config.adjustmentFajr,
    config.adjustmentSunrise,
    config.adjustmentDhuhr,
    config.adjustmentAsr,
    config.adjustmentMaghrib,
    config.adjustmentIsha
  );
}
