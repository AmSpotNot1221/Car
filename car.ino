#include <esp_now.h>
#include <WiFi.h>
#include <ESP32Servo.h>

#define PIN_LED 2
#define PIN_ESC 18
#define PIN_SERVO 19

#define SERVO_CENTER 90
#define MAX_STEER_DEFLECTION 4.0 

#define THROTTLE_STOP 1000       
#define THROTTLE_MAX 2000        
#define DECEL_ RATE 15     

typedef struct __attribute__((packed)) {
  int x;
  int y;
  bool button;
} ControlData;

ControlData joyData;
Servo escMotor;
Servo steeringServo;

int currentThrottle = THROTTLE_STOP;
int targetThrottle = THROTTLE_STOP;
unsigned long lastPacketTime = 0;
unsigned long lastRampTime = 0;

void OnDataRecv(const esp_now_recv_info *info, const uint8_t *incomingData, int len) {
  memcpy(&joyData, incomingData, sizeof(joyData));
  lastPacketTime = millis();
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  escMotor.setPeriodHertz(50);
  steeringServo.setPeriodHertz(50);

  escMotor.attach(PIN_ESC, 1000, 2000);
  steeringServo.attach(PIN_SERVO, 500, 2500);

  escMotor.writeMicroseconds(THROTTLE_STOP);
  steeringServo.write(SERVO_CENTER);

  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("INIT Failed");
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  if (millis() - lastPacketTime > 500) {
    joyData.button = false;
    targetThrottle = THROTTLE_STOP;
  }

  if (joyData.button) {
    digitalWrite(PIN_LED, HIGH);

    if (joyData.y < 1800) {
      targetThrottle = map(joyData.y, 1800, 0, THROTTLE_STOP, THROTTLE_MAX);
    } else {
      targetThrottle = THROTTLE_STOP;
    }

    float steerOffset = map(joyData.x, 0, 4095, -MAX_STEER_DEFLECTION, MAX_STEER_DEFLECTION);
    steeringServo.write(SERVO_CENTER + steerOffset);

  } else {
    digitalWrite(PIN_LED, LOW);
    targetThrottle = THROTTLE_STOP;
    steeringServo.write(SERVO_CENTER);
  }

  if (millis() - lastRampTime >= 10) {
    lastRampTime = millis();

    if (targetThrottle > currentThrottle) {
      currentThrottle = targetThrottle; 
    } else if (targetThrottle < currentThrottle) {
      currentThrottle -= DECEL_RATE;
      if (currentThrottle < targetThrottle) {
        currentThrottle = targetThrottle;
      }
    }
    escMotor.writeMicroseconds(currentThrottle);
  }
}