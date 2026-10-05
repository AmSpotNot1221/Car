#include <esp_now.h>
#include <WiFi.h>

// Badal dena isseee !!!!!!!
uint8_t carAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Isse bhi badalna !!!!
#define PIN_JOY_X 33
#define PIN_JOY_Y 32
#define PIN_JOY_SW 25

typedef struct __attribute__((packed)) {
  int x;          
  int y;          
  bool button;    
} ControlData;

ControlData joyData;
esp_now_peer_info_t carInfo;

bool lastButtonState = HIGH;
bool isCarOn = false;

void setup() {
  Serial.begin(115200);
  pinMode(PIN_JOY_SW, INPUT_PULLUP);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("INIT Failed");
    return;
  }

  memcpy(peerInfo.peer_addr, carAddress, 6);
  carInfo.channel = 0;  
  carInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to connect car");
    return;
  }
}

void loop() {
  joyData.x = analogRead(PIN_JOY_X);
  joyData.y = analogRead(PIN_JOY_Y);

  bool currentButtonState = digitalRead(PIN_JOY_SW);
  if (lastButtonState == HIGH && currentButtonState == LOW) {
    isCarOn = !isCarOn;
    delay(50);
  }
  lastButtonState = currentButtonState;
  joyData.button = isArmed;

  esp_now_send(carAddress, (uint8_t *)&joyData, sizeof(joyData));
  delay(20);
}