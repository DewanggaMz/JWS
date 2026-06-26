#include "connection.h"

#include <Arduino.h>
#include <WiFi.h>

#include "config.h"

void connectToWiFi() {
  // Serial.printf("Menghubungkan ke WiFi: %s\n", WIFI_SSID);
  // WiFi.mode(WIFI_STA);
  // WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(500);
  //   Serial.print(".");
  // }

  // Serial.println();
  // Serial.print("WiFi terhubung. IP address: ");
  // Serial.println(WiFi.localIP());
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.softAP("esp32", "11223344");
  unsigned long start = millis();

  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - start > 15000) {
      Serial.println("Gagal mendapatkan IP Address");
      return;
    }
    delay(100);
  }

  if(!WiFi.localIP() || !WiFi.softAPIP()) {
    Serial.println("Gagal mendapatkan IP Address");
    return;
  }

  Serial.print("WiFi terhubung. IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("SoftAP IP address: ");
  Serial.println(WiFi.softAPIP());
}
