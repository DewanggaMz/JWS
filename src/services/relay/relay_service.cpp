#include "services/relay/relay_service.h"

#include <atomic>
#include <string.h>

#include "config.h"
#include "services/database/database_service.h"

namespace {

const bool DEFAULT_ENABLED = true;
const uint16_t DEFAULT_PRE_PRAYER_MINUTES = 7;
const uint16_t DEFAULT_FRIDAY_PRE_PRAYER_MINUTES = 40;
const uint16_t DEFAULT_RELAY_12_ON_DELAY_SECONDS = 2;
const uint16_t DEFAULT_RELAY_12_OFF_DELAY_MINUTES = 5;
const bool DEFAULT_TARTIL_SUBUH = true;
const bool DEFAULT_TARTIL_DZUHUR = true;
const bool DEFAULT_TARTIL_JUMAT = true;
const bool DEFAULT_TARTIL_ASHAR = true;
const bool DEFAULT_TARTIL_MAGRIB = true;
const bool DEFAULT_TARTIL_ISHA = true;

const uint16_t MAX_PRE_PRAYER_MINUTES = 180;
const uint16_t MAX_RELAY_12_OFF_DELAY_MINUTES = 180;

RelayConfig activeConfig{
  DEFAULT_ENABLED,
  DEFAULT_PRE_PRAYER_MINUTES,
  DEFAULT_FRIDAY_PRE_PRAYER_MINUTES,
  DEFAULT_RELAY_12_ON_DELAY_SECONDS,
  DEFAULT_RELAY_12_OFF_DELAY_MINUTES,
  DEFAULT_TARTIL_SUBUH,
  DEFAULT_TARTIL_DZUHUR,
  DEFAULT_TARTIL_JUMAT,
  DEFAULT_TARTIL_ASHAR,
  DEFAULT_TARTIL_MAGRIB,
  DEFAULT_TARTIL_ISHA
};
PrayerSchedule activeSchedule{};
bool schedulerInitialized = false;
bool relay1On = false;
bool relay2On = false;
bool relay3On = false;
bool relay4On = false;
std::atomic<bool> relayConfigReloadRequested(false);

bool setBoolIfMissing(JsonObject object, const char *key, bool value)
{
  if (object[key].is<bool>()) {
    return false;
  }

  object[key] = value;
  return true;
}

bool setIntegerIfInvalid(
  JsonObject object,
  const char *key,
  uint16_t value,
  uint16_t maximum
)
{
  if (object[key].is<int>()) {
    const int current = object[key].as<int>();
    if (current >= 0 && current <= maximum) {
      return false;
    }
  }

  object[key] = value;
  return true;
}

bool parseRelayConfig(
  JsonVariantConst source,
  RelayConfig &config,
  String &message
)
{
  if (!source.is<JsonObjectConst>()) {
    message = "Konfigurasi relay harus berupa object";
    return false;
  }

  JsonObjectConst object = source.as<JsonObjectConst>();
  if (!object["enabled"].is<bool>() ||
      !object["prePrayerMinutes"].is<int>() ||
      !object["fridayPrePrayerMinutes"].is<int>() ||
      !object["relay12OnDelaySeconds"].is<int>() ||
      !object["relay12OffDelayMinutes"].is<int>() ||
      !object["tartilSubuh"].is<bool>() ||
      !object["tartilDzuhur"].is<bool>() ||
      !object["tartilJumat"].is<bool>() ||
      !object["tartilAshar"].is<bool>() ||
      !object["tartilMagrib"].is<bool>() ||
      !object["tartilIsha"].is<bool>()) {
    message = "Tipe data konfigurasi relay tidak valid";
    return false;
  }

  const int prePrayerMinutes = object["prePrayerMinutes"].as<int>();
  const int fridayPrePrayerMinutes =
    object["fridayPrePrayerMinutes"].as<int>();
  const int relay12OnDelaySeconds =
    object["relay12OnDelaySeconds"].as<int>();
  const int relay12OffDelayMinutes =
    object["relay12OffDelayMinutes"].as<int>();

  if (prePrayerMinutes < 1 ||
      prePrayerMinutes > MAX_PRE_PRAYER_MINUTES) {
    message = "prePrayerMinutes harus antara 1 sampai 180";
    return false;
  }

  if (fridayPrePrayerMinutes <= prePrayerMinutes ||
      fridayPrePrayerMinutes > MAX_PRE_PRAYER_MINUTES) {
    message =
      "fridayPrePrayerMinutes harus lebih besar dari prePrayerMinutes dan maksimal 180";
    return false;
  }

  if (relay12OnDelaySeconds < 0 ||
      relay12OnDelaySeconds > prePrayerMinutes * 60) {
    message =
      "relay12OnDelaySeconds harus antara 0 dan durasi sebelum sholat";
    return false;
  }

  if (relay12OffDelayMinutes < 0 ||
      relay12OffDelayMinutes > MAX_RELAY_12_OFF_DELAY_MINUTES) {
    message = "relay12OffDelayMinutes harus antara 0 sampai 180";
    return false;
  }

  config.enabled = object["enabled"].as<bool>();
  config.prePrayerMinutes =
    static_cast<uint16_t>(prePrayerMinutes);
  config.fridayPrePrayerMinutes =
    static_cast<uint16_t>(fridayPrePrayerMinutes);
  config.relay12OnDelaySeconds =
    static_cast<uint16_t>(relay12OnDelaySeconds);
  config.relay12OffDelayMinutes =
    static_cast<uint16_t>(relay12OffDelayMinutes);
  config.tartilSubuh = object["tartilSubuh"].as<bool>();
  config.tartilDzuhur = object["tartilDzuhur"].as<bool>();
  config.tartilJumat = object["tartilJumat"].as<bool>();
  config.tartilAshar = object["tartilAshar"].as<bool>();
  config.tartilMagrib = object["tartilMagrib"].as<bool>();
  config.tartilIsha = object["tartilIsha"].as<bool>();
  return true;
}

void writeRelay(
  int pin,
  uint8_t channel,
  bool on,
  bool &currentState
)
{
  if (currentState == on) {
    return;
  }

  currentState = on;
  digitalWrite(pin, on ? LOW : HIGH);
  Serial.printf("Relay %u %s\n", channel, on ? "ON" : "OFF");
}

void switchAllRelaysOff()
{
  writeRelay(PIN_RELAY_1, 1, false, relay1On);
  writeRelay(PIN_RELAY_2, 2, false, relay2On);
  writeRelay(PIN_RELAY_3, 3, false, relay3On);
  writeRelay(PIN_RELAY_4, 4, false, relay4On);
}

bool isWithinWindow(
  int currentSecond,
  int startSecond,
  int endSecond
)
{
  const int boundedStart = max(0, startSecond);
  const int boundedEnd = min(86400, endSecond);
  return boundedStart < boundedEnd &&
         currentSecond >= boundedStart &&
         currentSecond < boundedEnd;
}

void includePrayerWindow(
  const PrayerScheduleTime &prayer,
  int currentSecond,
  bool &shouldRelay12BeOn,
  bool &shouldRelay4BeOn
)
{
  if (!prayer.valid) {
    return;
  }

  const int prayerSecond =
    (static_cast<int>(prayer.hour) * 3600) +
    (static_cast<int>(prayer.minute) * 60);
  const int relay4Start =
    prayerSecond -
    (static_cast<int>(activeConfig.prePrayerMinutes) * 60);
  const int relay12Start =
    relay4Start +
    static_cast<int>(activeConfig.relay12OnDelaySeconds);
  const int relay12End =
    prayerSecond +
    (static_cast<int>(activeConfig.relay12OffDelayMinutes) * 60);

  shouldRelay4BeOn |= isWithinWindow(
    currentSecond,
    relay4Start,
    prayerSecond
  );
  shouldRelay12BeOn |= isWithinWindow(
    currentSecond,
    relay12Start,
    relay12End
  );
}

void includeFridayDhuhrWindow(
  const PrayerScheduleTime &dhuhr,
  int currentSecond,
  bool &shouldRelay12BeOn,
  bool &shouldRelay3BeOn,
  bool &shouldRelay4BeOn
)
{
  if (!dhuhr.valid) {
    return;
  }

  const int prayerSecond =
    (static_cast<int>(dhuhr.hour) * 3600) +
    (static_cast<int>(dhuhr.minute) * 60);
  const int relay3Start =
    prayerSecond -
    (static_cast<int>(activeConfig.fridayPrePrayerMinutes) * 60);
  const int relay4Start =
    prayerSecond -
    (static_cast<int>(activeConfig.prePrayerMinutes) * 60);
  const int relay12Start =
    relay3Start +
    static_cast<int>(activeConfig.relay12OnDelaySeconds);

  shouldRelay3BeOn |= isWithinWindow(
    currentSecond,
    relay3Start,
    relay4Start
  );
  shouldRelay4BeOn |= isWithinWindow(
    currentSecond,
    relay4Start,
    prayerSecond
  );
  shouldRelay12BeOn |= isWithinWindow(
    currentSecond,
    relay12Start,
    prayerSecond
  );
}

void reloadRelayConfigIfRequested()
{
  if (!relayConfigReloadRequested.exchange(
        false,
        std::memory_order_acq_rel
      )) {
    return;
  }

  RelayConfig config;
  if (loadRelayConfig(config)) {
    activeConfig = config;
    Serial.println("Konfigurasi relay berhasil dimuat ulang");
  } else {
    Serial.println("Gagal memuat ulang konfigurasi relay");
  }
}

}

void setupRelayPins()
{
  digitalWrite(PIN_RELAY_1, HIGH);
  digitalWrite(PIN_RELAY_2, HIGH);
  digitalWrite(PIN_RELAY_3, HIGH);
  digitalWrite(PIN_RELAY_4, HIGH);
  pinMode(PIN_RELAY_1, OUTPUT);
  pinMode(PIN_RELAY_2, OUTPUT);
  pinMode(PIN_RELAY_3, OUTPUT);
  pinMode(PIN_RELAY_4, OUTPUT);

  relay1On = false;
  relay2On = false;
  relay3On = false;
  relay4On = false;
}

bool ensureRelayConfig()
{
  DatabaseGuard guard;
  if (!guard) {
    return false;
  }

  JsonDocument database;
  if (!loadDatabase(database)) {
    return false;
  }

  bool changed = false;
  if (!database["relayConfig"].is<JsonObject>()) {
    database["relayConfig"].to<JsonObject>();
    changed = true;
  }

  JsonObject config = database["relayConfig"].as<JsonObject>();
  changed |= setBoolIfMissing(config, "enabled", DEFAULT_ENABLED);
  changed |= setIntegerIfInvalid(
    config,
    "prePrayerMinutes",
    DEFAULT_PRE_PRAYER_MINUTES,
    MAX_PRE_PRAYER_MINUTES
  );
  changed |= setIntegerIfInvalid(
    config,
    "fridayPrePrayerMinutes",
    DEFAULT_FRIDAY_PRE_PRAYER_MINUTES,
    MAX_PRE_PRAYER_MINUTES
  );
  changed |= setIntegerIfInvalid(
    config,
    "relay12OnDelaySeconds",
    DEFAULT_RELAY_12_ON_DELAY_SECONDS,
    MAX_PRE_PRAYER_MINUTES * 60
  );
  changed |= setIntegerIfInvalid(
    config,
    "relay12OffDelayMinutes",
    DEFAULT_RELAY_12_OFF_DELAY_MINUTES,
    MAX_RELAY_12_OFF_DELAY_MINUTES
  );
  changed |= setBoolIfMissing(
    config,
    "tartilSubuh",
    DEFAULT_TARTIL_SUBUH
  );
  changed |= setBoolIfMissing(
    config,
    "tartilDzuhur",
    DEFAULT_TARTIL_DZUHUR
  );
  changed |= setBoolIfMissing(
    config,
    "tartilJumat",
    DEFAULT_TARTIL_JUMAT
  );
  changed |= setBoolIfMissing(
    config,
    "tartilAshar",
    DEFAULT_TARTIL_ASHAR
  );
  changed |= setBoolIfMissing(
    config,
    "tartilMagrib",
    DEFAULT_TARTIL_MAGRIB
  );
  changed |= setBoolIfMissing(
    config,
    "tartilIsha",
    DEFAULT_TARTIL_ISHA
  );

  RelayConfig parsed;
  String message;
  if (!parseRelayConfig(config, parsed, message)) {
    config["enabled"] = DEFAULT_ENABLED;
    config["prePrayerMinutes"] = DEFAULT_PRE_PRAYER_MINUTES;
    config["fridayPrePrayerMinutes"] =
      DEFAULT_FRIDAY_PRE_PRAYER_MINUTES;
    config["relay12OnDelaySeconds"] =
      DEFAULT_RELAY_12_ON_DELAY_SECONDS;
    config["relay12OffDelayMinutes"] =
      DEFAULT_RELAY_12_OFF_DELAY_MINUTES;
    config["tartilSubuh"] = DEFAULT_TARTIL_SUBUH;
    config["tartilDzuhur"] = DEFAULT_TARTIL_DZUHUR;
    config["tartilJumat"] = DEFAULT_TARTIL_JUMAT;
    config["tartilAshar"] = DEFAULT_TARTIL_ASHAR;
    config["tartilMagrib"] = DEFAULT_TARTIL_MAGRIB;
    config["tartilIsha"] = DEFAULT_TARTIL_ISHA;
    changed = true;
  }

  return !changed || saveDatabase(database);
}

bool loadRelayConfig(RelayConfig &config)
{
  JsonDocument database;
  if (!loadDatabase(database)) {
    return false;
  }

  String message;
  return parseRelayConfig(database["relayConfig"], config, message);
}

bool beginRelayScheduler(const PrayerSchedule &schedule)
{
  RelayConfig config;
  if (!loadRelayConfig(config)) {
    switchAllRelaysOff();
    return false;
  }

  activeConfig = config;
  activeSchedule = schedule;
  schedulerInitialized = true;
  return true;
}

void setRelayPrayerSchedule(const PrayerSchedule &schedule)
{
  activeSchedule = schedule;
}

void relayLoop(const Time &now, const Date &today)
{
  reloadRelayConfigIfRequested();

  if (!schedulerInitialized ||
      !activeConfig.enabled ||
      !activeSchedule.valid) {
    switchAllRelaysOff();
    return;
  }

  const int currentSecond =
    (static_cast<int>(now.hour) * 3600) +
    (static_cast<int>(now.minute) * 60) +
    now.second;
  bool shouldRelay12BeOn = false;
  bool shouldRelay3BeOn = false;
  bool shouldRelay4BeOn = false;
  const bool isFriday =
    today.dayName != nullptr &&
    strcmp(today.dayName, "Jumat") == 0;

  if (activeConfig.tartilSubuh) {
    includePrayerWindow(
      activeSchedule.subuh,
      currentSecond,
      shouldRelay12BeOn,
      shouldRelay4BeOn
    );
  }
  if (isFriday) {
    if (activeConfig.tartilJumat) {
      includeFridayDhuhrWindow(
        activeSchedule.dzuhur,
        currentSecond,
        shouldRelay12BeOn,
        shouldRelay3BeOn,
        shouldRelay4BeOn
      );
    }
  } else {
    if (activeConfig.tartilDzuhur) {
      includePrayerWindow(
        activeSchedule.dzuhur,
        currentSecond,
        shouldRelay12BeOn,
        shouldRelay4BeOn
      );
    }
  }
  if (activeConfig.tartilAshar) {
    includePrayerWindow(
      activeSchedule.ashar,
      currentSecond,
      shouldRelay12BeOn,
      shouldRelay4BeOn
    );
  }
  if (activeConfig.tartilMagrib) {
    includePrayerWindow(
      activeSchedule.maghrib,
      currentSecond,
      shouldRelay12BeOn,
      shouldRelay4BeOn
    );
  }
  if (activeConfig.tartilIsha) {
    includePrayerWindow(
      activeSchedule.isya,
      currentSecond,
      shouldRelay12BeOn,
      shouldRelay4BeOn
    );
  }

  writeRelay(
    PIN_RELAY_4,
    4,
    shouldRelay4BeOn,
    relay4On
  );
  writeRelay(
    PIN_RELAY_1,
    1,
    shouldRelay12BeOn,
    relay1On
  );
  writeRelay(
    PIN_RELAY_2,
    2,
    shouldRelay12BeOn,
    relay2On
  );
  writeRelay(
    PIN_RELAY_3,
    3,
    shouldRelay3BeOn,
    relay3On
  );
}

bool updateRelayConfig(
  JsonVariantConst payload,
  RelayConfig &config,
  String &message
)
{
  DatabaseGuard guard;
  if (!guard) {
    message = "Database sedang digunakan";
    return false;
  }

  if (!payload.is<JsonObjectConst>()) {
    message = "Payload konfigurasi relay harus berupa object";
    return false;
  }

  JsonObjectConst input = payload.as<JsonObjectConst>();
  const bool hasEnabled = !input["enabled"].isUnbound();
  const bool hasPrePrayerMinutes =
    !input["prePrayerMinutes"].isUnbound();
  const bool hasFridayPrePrayerMinutes =
    !input["fridayPrePrayerMinutes"].isUnbound();
  const bool hasRelay12OnDelaySeconds =
    !input["relay12OnDelaySeconds"].isUnbound();
  const bool hasRelay12OffDelayMinutes =
    !input["relay12OffDelayMinutes"].isUnbound();
  if (!hasEnabled &&
      !hasPrePrayerMinutes &&
      !hasFridayPrePrayerMinutes &&
      !hasRelay12OnDelaySeconds &&
      !hasRelay12OffDelayMinutes) {
    message = "Tidak ada field konfigurasi relay yang dikenali";
    return false;
  }

  if (hasEnabled && !input["enabled"].is<bool>()) {
    message = "Field enabled harus berupa boolean";
    return false;
  }
  if (hasPrePrayerMinutes &&
      !input["prePrayerMinutes"].is<int>()) {
    message = "Field prePrayerMinutes harus berupa angka";
    return false;
  }
  if (hasFridayPrePrayerMinutes &&
      !input["fridayPrePrayerMinutes"].is<int>()) {
    message = "Field fridayPrePrayerMinutes harus berupa angka";
    return false;
  }
  if (hasRelay12OnDelaySeconds &&
      !input["relay12OnDelaySeconds"].is<int>()) {
    message = "Field relay12OnDelaySeconds harus berupa angka";
    return false;
  }
  if (hasRelay12OffDelayMinutes &&
      !input["relay12OffDelayMinutes"].is<int>()) {
    message = "Field relay12OffDelayMinutes harus berupa angka";
    return false;
  }

  JsonDocument database;
  if (!loadDatabase(database)) {
    message = "Gagal membaca database";
    return false;
  }
  const bool configExists = database["relayConfig"].is<JsonObject>();
  JsonObject stored = configExists
                        ? database["relayConfig"].as<JsonObject>()
                        : database["relayConfig"].to<JsonObject>();
  if (!configExists) {
    stored["enabled"] = DEFAULT_ENABLED;
    stored["prePrayerMinutes"] = DEFAULT_PRE_PRAYER_MINUTES;
    stored["fridayPrePrayerMinutes"] =
      DEFAULT_FRIDAY_PRE_PRAYER_MINUTES;
    stored["relay12OnDelaySeconds"] =
      DEFAULT_RELAY_12_ON_DELAY_SECONDS;
    stored["relay12OffDelayMinutes"] =
      DEFAULT_RELAY_12_OFF_DELAY_MINUTES;
    stored["tartilSubuh"] = DEFAULT_TARTIL_SUBUH;
    stored["tartilDzuhur"] = DEFAULT_TARTIL_DZUHUR;
    stored["tartilJumat"] = DEFAULT_TARTIL_JUMAT;
    stored["tartilAshar"] = DEFAULT_TARTIL_ASHAR;
    stored["tartilMagrib"] = DEFAULT_TARTIL_MAGRIB;
    stored["tartilIsha"] = DEFAULT_TARTIL_ISHA;
  }

  if (hasEnabled) {
    stored["enabled"] = input["enabled"].as<bool>();
  }
  if (hasPrePrayerMinutes) {
    stored["prePrayerMinutes"] =
      input["prePrayerMinutes"].as<int>();
  }
  if (hasFridayPrePrayerMinutes) {
    stored["fridayPrePrayerMinutes"] =
      input["fridayPrePrayerMinutes"].as<int>();
  }
  if (hasRelay12OnDelaySeconds) {
    stored["relay12OnDelaySeconds"] =
      input["relay12OnDelaySeconds"].as<int>();
  }
  if (hasRelay12OffDelayMinutes) {
    stored["relay12OffDelayMinutes"] =
      input["relay12OffDelayMinutes"].as<int>();
  }

  if (!parseRelayConfig(stored, config, message)) {
    return false;
  }
  if (!saveDatabase(database)) {
    message = "Gagal menyimpan konfigurasi relay";
    return false;
  }

  message = "Konfigurasi relay berhasil diperbarui";
  return true;
}

bool updateRelayPrayerStates(
  JsonVariantConst payload,
  RelayConfig &config,
  String &message
)
{
  DatabaseGuard guard;
  if (!guard) {
    message = "Database sedang digunakan";
    return false;
  }

  if (!payload.is<JsonObjectConst>()) {
    message = "Payload state tartil harus berupa object";
    return false;
  }

  JsonObjectConst input = payload.as<JsonObjectConst>();
  const bool hasSubuh = !input["tartilSubuh"].isUnbound();
  const bool hasDzuhur = !input["tartilDzuhur"].isUnbound();
  const bool hasJumat = !input["tartilJumat"].isUnbound();
  const bool hasAshar = !input["tartilAshar"].isUnbound();
  const bool hasMagrib = !input["tartilMagrib"].isUnbound();
  const bool hasIsha = !input["tartilIsha"].isUnbound();
  if (!hasSubuh &&
      !hasDzuhur &&
      !hasJumat &&
      !hasAshar &&
      !hasMagrib &&
      !hasIsha) {
    message = "Tidak ada field state tartil yang dikenali";
    return false;
  }

  if ((hasSubuh && !input["tartilSubuh"].is<bool>()) ||
      (hasDzuhur && !input["tartilDzuhur"].is<bool>()) ||
      (hasJumat && !input["tartilJumat"].is<bool>()) ||
      (hasAshar && !input["tartilAshar"].is<bool>()) ||
      (hasMagrib && !input["tartilMagrib"].is<bool>()) ||
      (hasIsha && !input["tartilIsha"].is<bool>())) {
    message = "Semua state tartil harus berupa boolean";
    return false;
  }

  JsonDocument database;
  if (!loadDatabase(database)) {
    message = "Gagal membaca database";
    return false;
  }
  if (!database["relayConfig"].is<JsonObject>()) {
    message = "Konfigurasi relay belum tersedia";
    return false;
  }

  JsonObject stored = database["relayConfig"].as<JsonObject>();
  if (hasSubuh) {
    stored["tartilSubuh"] = input["tartilSubuh"].as<bool>();
  }
  if (hasDzuhur) {
    stored["tartilDzuhur"] = input["tartilDzuhur"].as<bool>();
  }
  if (hasJumat) {
    stored["tartilJumat"] = input["tartilJumat"].as<bool>();
  }
  if (hasAshar) {
    stored["tartilAshar"] = input["tartilAshar"].as<bool>();
  }
  if (hasMagrib) {
    stored["tartilMagrib"] = input["tartilMagrib"].as<bool>();
  }
  if (hasIsha) {
    stored["tartilIsha"] = input["tartilIsha"].as<bool>();
  }

  if (!parseRelayConfig(stored, config, message)) {
    return false;
  }
  if (!saveDatabase(database)) {
    message = "Gagal menyimpan state tartil";
    return false;
  }

  message = "State tartil berhasil diperbarui";
  return true;
}

void requestRelayConfigReload()
{
  relayConfigReloadRequested.store(true, std::memory_order_release);
}
