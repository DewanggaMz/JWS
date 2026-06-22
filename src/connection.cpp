#include "connection.h"

#include <Arduino.h>
#include <WiFi.h>

#include "config.h"

void connectToWiFi() {
  Serial.printf("Menghubungkan ke WiFi: %s\n", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("WiFi terhubung. IP address: ");
  Serial.println(WiFi.localIP());
}
