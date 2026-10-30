#include <WiFi.h>
#include <esp_now.h>

// ==========================================
// ESP-NOW & MOTOR CONFIGURATION
// ==========================================
typedef struct {
  int8_t speed;   
  uint8_t enable; 
} Cmd;

Cmd cmd;
volatile uint32_t lastRx = 0;

// BTS7960 Motor Driver Pins (Change to match your wiring)
// NOTE: If the slave motors spin the wrong way because it is 
// physically mirrored, swap the RPWM and LPWM pin numbers here!
const int RPWM = 25;
const int LPWM = 26;
const int R_EN = 27;
const int L_EN = 14;

void setupMotors() {
  pinMode(RPWM, OUTPUT);
  pinMode(LPWM, OUTPUT);
  pinMode(R_EN, OUTPUT);
  pinMode(L_EN, OUTPUT);
  digitalWrite(R_EN, HIGH);
  digitalWrite(L_EN, HIGH);
}

void driveMotors(int8_t speed, uint8_t enable) {
  if (!enable || speed == 0) {
    analogWrite(RPWM, 0);
    analogWrite(LPWM, 0);
    return;
  }

  int pwmVal = map(abs(speed), 0, 100, 0, 255);

  if (speed > 0) { 
    analogWrite(RPWM, pwmVal);
    analogWrite(LPWM, 0);
  } else {         
    analogWrite(RPWM, 0);
    analogWrite(LPWM, pwmVal);
  }
}

// Callback for receiving ESP-NOW data (ESP32 Core 3.x signature)
// If using older ESP32 core 2.x, change signature to: 
// void onRecv(const uint8_t* mac, const uint8_t* data, int len)
void onRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  if (len == sizeof(cmd)) {
    memcpy(&cmd, data, sizeof(cmd));
    lastRx = millis(); // Reset failsafe timer
  }
}

void setup() {
  Serial.begin(115200);
  
  // Print MAC Address so you can copy it to the Master code
  Serial.print("Slave MAC Address: ");
  Serial.println(WiFi.macAddress());

  setupMotors();
  
  WiFi.mode(WIFI_STA);
  
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Register receive callback
  esp_now_register_recv_cb(onRecv);
}

void loop() {
  // Failsafe: If no data received for 300ms, force stop
  if (millis() - lastRx > 300) {
    cmd.speed = 0;
    cmd.enable = 0;
  }
  
  // Drive Slave's motors continuously 
  driveMotors(cmd.speed, cmd.enable);
  
  delay(10);
}