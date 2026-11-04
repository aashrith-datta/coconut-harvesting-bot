// ESP32 Slave: receive motion commands over ESP-NOW and control motor

#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

#define RPWM 18
#define LPWM 19
#define R_EN 21
#define L_EN 22

#define WIFI_CHANNEL 1

const int SPEED = 255;  // 0–255

enum MotionCommand : uint8_t {
  STOP = 0,
  MOVE_UP = 1,
  MOVE_DOWN = 2
};

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

void OnDataRecv(const esp_now_recv_info_t *info,
                const uint8_t *incomingData,
                int len) {
  if (len != sizeof(uint8_t)) return;

  uint8_t command;
  memcpy(&command, incomingData, sizeof(command));

  switch (command) {
    case MOVE_UP:
      up();
      Serial.println("Received command: UP");
      break;

    case MOVE_DOWN:
      down();
      Serial.println("Received command: DOWN");
      break;

    case STOP:
      stopMotor();
      Serial.println("Received command: STOP");
      break;

    default:
      stopMotor();
      Serial.println("Received invalid command; motor stopped");
      break;
  }
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

  WiFi.mode(WIFI_STA);
  esp_wifi_set_channel(WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE);

  Serial.print("ESP32 #2 MAC: ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW initialization failed!");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);
  Serial.println("ESP32 #2 Ready!");
}

void loop() {
}