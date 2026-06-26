#include "connection.h"

#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>

#include "config.h"

void connectToWiFi() {
  WiFi.persistent(false);
  WiFi.setAutoReconnect(true);
  WiFi.setSleep(false);
  esp_wifi_set_ps(WIFI_PS_NONE);

  WiFi.mode(WIFI_ENABLE_AP_STA ? WIFI_AP_STA : WIFI_STA);
  WiFi.setTxPower(WIFI_POWER_11dBm);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  if (WIFI_ENABLE_AP_STA) {
    WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD);
  }

  unsigned long start = millis();

  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - start > WIFI_CONNECT_TIMEOUT_MS) {
      Serial.println("Gagal terhubung ke WiFi STA");
      break;
    }
    delay(50);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("WiFi STA terhubung. IP address: ");
    Serial.println(WiFi.localIP());
  } else if (WIFI_ENABLE_AP_FALLBACK && !WIFI_ENABLE_AP_STA) {
    WiFi.mode(WIFI_AP);
    WiFi.setSleep(false);
    esp_wifi_set_ps(WIFI_PS_NONE);
    WiFi.setTxPower(WIFI_POWER_11dBm);

    if (!WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD)) {
      Serial.println("Gagal menyalakan SoftAP");
      return;
    }

    Serial.print("SoftAP aktif. IP address: ");
    Serial.println(WiFi.softAPIP());
  } else {
    return;
  }

  if (WIFI_ENABLE_AP_STA) {
    Serial.print("SoftAP IP address: ");
    Serial.println(WiFi.softAPIP());
  }
}
