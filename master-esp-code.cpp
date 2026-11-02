//ESP32 Master
// RemoteXY BLE configuration
#define REMOTEXY_MODE__ESP32CORE_BLE
#define REMOTEXY_BLUETOOTH_NAME "CocoClimber Master"

#include <BLEDevice.h>
#include <RemoteXY.h>

#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

#define LED_PIN 2
#define WIFI_CHANNEL 1

// ESP32 #2 MAC ADDRESS
uint8_t ESP32_2_MAC[] = {
  0x68, 0x09, 0x47, 0x60, 0x49, 0x98
};

// RemoteXY GUI configuration
#pragma pack(push, 1)

uint8_t const PROGMEM RemoteXY_CONF_PROGMEM[] = {
  255,1,0,0,0,30,0,19,0,0,0,0,31,1,106,200,
  1,1,1,0,2,26,77,57,33,0,2,26,31,31,79,78,
  0,79,70,70,0
};

struct {

  uint8_t switch_01;
  uint8_t connect_flag;

} RemoteXY;

#pragma pack(pop)

// Previous switch state
uint8_t previousState = 255;

void setup() {

  Serial.begin(115200);

  // Initialize onboard LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Initialize RemoteXY BLE
  RemoteXY_Init();

  Serial.println("RemoteXY BLE initialized!");

  // Initialize Wi-Fi
  WiFi.mode(WIFI_STA);

  // Set ESP-NOW channel
  esp_wifi_set_channel(WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE);

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW initialization failed!");
    return;
  }

  // Configure ESP32 #2 as peer
  esp_now_peer_info_t peerInfo = {};

  memcpy(peerInfo.peer_addr, ESP32_2_MAC, 6);

  peerInfo.channel = WIFI_CHANNEL;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add ESP32 #2!");
    return;
  }

  Serial.println("ESP32 #1 Ready!");
}

void loop() {

  // Handle RemoteXY communication
  RemoteXYEngine.handler();

  // Read switch state
  uint8_t currentState = RemoteXY.switch_01;

  // Detect switch state change
  if (currentState != previousState) {

    previousState = currentState;

    // Control ESP32 #1 LED
    digitalWrite(LED_PIN, currentState ? HIGH : LOW);

    // Send command to ESP32 #2
    esp_err_t result = esp_now_send(
      ESP32_2_MAC,
      &currentState,
      sizeof(currentState)
    );

    Serial.print("Switch state: ");
    Serial.println(currentState ? "ON" : "OFF");

    if (result == ESP_OK) {
      Serial.println("ESP-NOW transmission initiated!");
    }
    else {
      Serial.println("ESP-NOW transmission failed!");
    }
  }
}