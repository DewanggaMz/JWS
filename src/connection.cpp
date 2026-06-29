#include "connection.h"

#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>

#include "config.h"
#include "services/wifi_config/wifi_config_service.h"

namespace {

bool startAccessPoint(const WiFiConfig &config)
{
  WiFi.mode(WIFI_AP);
  WiFi.setSleep(false);
  WiFi.setTxPower(WIFI_POWER_11dBm);
  esp_wifi_set_ps(WIFI_PS_NONE);

  if (!WiFi.softAP(config.ssid.c_str(), config.password.c_str())) {
    Serial.println("Gagal menyalakan WiFi mode AP");
    return false;
  }

  Serial.printf("WiFi AP aktif. SSID: %s\n", config.ssid.c_str());
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());
  return true;
}

bool connectStation(const WiFiConfig &config)
{
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.setSleep(false);
  WiFi.setTxPower(WIFI_POWER_11dBm);
  esp_wifi_set_ps(WIFI_PS_NONE);
  WiFi.begin(config.ssid.c_str(), config.password.c_str());

  Serial.printf("Menghubungkan WiFi STA ke SSID: %s", config.ssid.c_str());
  const unsigned long startedAt = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - startedAt >= WIFI_STA_CONNECT_TIMEOUT_MS) {
      Serial.println("\nGagal terhubung ke WiFi STA: timeout");
      WiFi.disconnect(true);
      return false;
    }

    delay(250);
    Serial.print(".");
  }

  Serial.println("\nWiFi STA terhubung");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.printf("Signal strength: %d dBm\n", WiFi.RSSI());
  return true;
}

}

bool connectToWiFi()
{
  WiFiConfig config;
  if (!loadWiFiConfig(config)) {
    Serial.println("Konfigurasi WiFi tidak valid, menggunakan konfigurasi default");
  }

  WiFi.persistent(false);

  if (config.mode == "STA") {
    return connectStation(config);
  }

  return startAccessPoint(config);
}
