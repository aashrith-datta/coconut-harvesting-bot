#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  delay(1000); // Allow hardware power to stabilize

  WiFi.mode(WIFI_STA); // Forces Wi-Fi hardware radio ON
  delay(100);          // Allow Wi-Fi stack to initialize

  Serial.print("Valid MAC Address: ");
  Serial.println(WiFi.macAddress());
}

void loop() {}