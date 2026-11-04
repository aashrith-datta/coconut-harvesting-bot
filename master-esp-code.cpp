// ESP32 Master: RemoteXY BLE + ESP-NOW + motor control

#define REMOTEXY_MODE__ESP32CORE_BLE
#define REMOTEXY_BLUETOOTH_NAME "CocoClimber Master"

#include <BLEDevice.h>
#include <RemoteXY.h>

#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

#define RPWM 18
#define LPWM 19
#define R_EN 21
#define L_EN 22

#define WIFI_CHANNEL 1

const int SPEED = 255;  // 0–255

// ESP32 #2 MAC address
uint8_t ESP32_2_MAC[] = {
  0x68, 0x09, 0x47, 0x60, 0x49, 0x98
};

// RemoteXY interface configuration
#pragma pack(push, 1)

uint8_t const PROGMEM RemoteXY_CONF_PROGMEM[] = {
  255,2,0,0,0,53,0,19,0,0,0,0,31,1,106,200,1,1,4,0,
  1,41,57,26,26,0,15,31,0,1,40,111,26,26,0,15,31,0,129,46,
  64,16,12,64,8,85,80,0,129,41,120,24,8,64,24,68,79,87,78,0
};

struct {
  uint8_t UP;          // 1 while pressed
  uint8_t DOWN;        // 1 while pressed
  uint8_t connect_flag;
} RemoteXY;

#pragma pack(pop)

// Commands sent to ESP32 #2
enum MotionCommand : uint8_t {
  STOP = 0,
  MOVE_UP = 1,
  MOVE_DOWN = 2
};

uint8_t previousCommand = 255;

void up() {
  ledcWrite(LPWM, 0);
  ledcWrite(RPWM, SPEED);
}

void down() {
  ledcWrite(RPWM, 0);
  ledcWrite(LPWM, SPEED);
}

void stopMotor() {
  ledcWrite(RPWM, 0);
  ledcWrite(LPWM, 0);
}

void setup() {
  Serial.begin(115200);

  pinMode(R_EN, OUTPUT);
  pinMode(L_EN, OUTPUT);
  digitalWrite(R_EN, HIGH);
  digitalWrite(L_EN, HIGH);

  // ESP32 Arduino core 3.x LEDC API
  ledcAttach(RPWM, 20000, 8);
  ledcAttach(LPWM, 20000, 8);
  stopMotor();

  RemoteXY_Init();
  Serial.println("RemoteXY BLE initialized!");

  WiFi.mode(WIFI_STA);
  esp_wifi_set_channel(WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW initialization failed!");
    return;
  }

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
  RemoteXYEngine.handler();

  // If both buttons are pressed, stop for safety.
  uint8_t command = STOP;
  if (RemoteXY.UP && !RemoteXY.DOWN) {
    command = MOVE_UP;
  } else if (RemoteXY.DOWN && !RemoteXY.UP) {
    command = MOVE_DOWN;
  }

  // Only update the motor and transmit when the command changes.
  if (command != previousCommand) {
    previousCommand = command;

    if (command == MOVE_UP) {
      up();
      Serial.println("Moving UP");
    } else if (command == MOVE_DOWN) {
      down();
      Serial.println("Moving DOWN");
    } else {
      stopMotor();
      Serial.println("Stopped");
    }

    esp_err_t result = esp_now_send(
      ESP32_2_MAC,
      &command,
      sizeof(command)
    );

    if (result != ESP_OK) {
      Serial.println("ESP-NOW transmission failed!");
    }
  }
}