#define REMOTEXY_MODE__ESP32CORE_BLE
#include <BLEDevice.h>
#include <WiFi.h>
#include <esp_now.h>
#include <RemoteXY.h>

// ==========================================
// REMOTEXY CONFIGURATION (Replace with yours)
// ==========================================
#define REMOTEXY_BLUETOOTH_NAME "TreeBot"
#pragma pack(push, 1)
uint8_t RemoteXY_CONF[] =   // PASTE YOUR GENERATED CONFIG HERE
  { 255,2,0,0,0,38,0,19,0,0,0,0,31,1,106,200,1,1,2,0,
  2,29,37,44,22,0,2,26,31,31,79,78,0,79,70,70,0,4,44,97,
  14,36,48,2,26 };

struct {
  int8_t slider_01; // -100 to 100
  uint8_t switch_01; // 0 = off, 1 = on
  uint8_t connect_flag; 
} RemoteXY;
#pragma pack(pop)

// ==========================================
// ESP-NOW & MOTOR CONFIGURATION
// ==========================================
// REPLACE WITH YOUR SLAVE'S MAC ADDRESS
uint8_t slaveMac[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}; 

typedef struct {
  int8_t speed;   // -100..100
  uint8_t enable; // 0 or 1
} Cmd;

Cmd cmd;

// BTS7960 Motor Driver Pins (Change to match your wiring)
const int RPWM = 25;
const int LPWM = 26;
const int R_EN = 27;
const int L_EN = 14;

void setupMotors() {
  pinMode(RPWM, OUTPUT);
  pinMode(LPWM, OUTPUT);
  pinMode(R_EN, OUTPUT);
  pinMode(L_EN, OUTPUT);
  // Enable the motor driver
  digitalWrite(R_EN, HIGH);
  digitalWrite(L_EN, HIGH);
}

void driveMotors(int8_t speed, uint8_t enable) {
  if (!enable || speed == 0) {
    analogWrite(RPWM, 0);
    analogWrite(LPWM, 0);
    return;
  }

  // Map -100..100 to 0..255 PWM range
  int pwmVal = map(abs(speed), 0, 100, 0, 255);

  if (speed > 0) { // Up
    analogWrite(RPWM, pwmVal);
    analogWrite(LPWM, 0);
  } else {         // Down
    analogWrite(RPWM, 0);
    analogWrite(LPWM, pwmVal);
  }
}

void setup() {
  Serial.begin(115200);
  
  // 1. Initialize Motors
  setupMotors();
  
  // 2. Initialize WiFi for ESP-NOW (STA mode required)
  WiFi.mode(WIFI_STA);
  
  // 3. Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Register Slave peer
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, slaveMac, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  // 4. Initialize RemoteXY (Bluetooth)
  RemoteXY_Init();
}

void loop() {
  RemoteXY_Handler();
  
  // Populate command struct from RemoteXY UI
  cmd.enable = RemoteXY.switch_01;
  
  // Only apply speed if the arm switch is ON
  if (cmd.enable == 0) {
    cmd.speed = 0;
  } else {
    cmd.speed = RemoteXY.slider_01;
  }

  // Send command to Slave via ESP-NOW
  esp_now_send(slaveMac, (uint8_t *) &cmd, sizeof(cmd));
  
  // Drive Master's motors
  driveMotors(cmd.speed, cmd.enable);
  
  // Loop delay to send at ~50Hz
  delay(20); 
}