//ESP32 Slave 
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

#define LED_PIN 2
#define WIFI_CHANNEL 1

void OnDataRecv(const esp_now_recv_info_t *info,
                const uint8_t *incomingData,
                int len) {

  if (len != sizeof(uint8_t)) return;

  uint8_t receivedState;

  memcpy(&receivedState, incomingData, sizeof(receivedState));

  digitalWrite(LED_PIN, receivedState ? HIGH : LOW);

  Serial.print("Received command: ");
  Serial.println(receivedState ? "ON" : "OFF");
}

void setup() {

  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Initialize Wi-Fi
  WiFi.mode(WIFI_STA);

  // Set ESP-NOW channel
  esp_wifi_set_channel(WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE);

  Serial.print("ESP32 #2 MAC: ");
  Serial.println(WiFi.macAddress());

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW initialization failed!");
    return;
  }

  // Register receive callback
  esp_now_register_recv_cb(OnDataRecv);

  Serial.println("ESP32 #2 Ready!");
}

void loop() {

}