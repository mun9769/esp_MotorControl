#include <ModbusMaster.h>

ModbusMaster node;

#define MAX485_DE 4
#define MODBUS_ID 1

// Pin Setting
const int pulsePin = 26;  // 노란색
const int signPin = 25;   // 빨간색

// Parameter
int32_t signedPos = 0;
const float e_revolution = 131072;  // Encoder : 1바퀴당 Pulse (=2^17)
const float d_revolution = 360;

// 모드별 각도 설정
const float O = 0;
const float Z = 49.3;  // Zero Trun : arctan(1100/950)
const float C = 90;    // Crab Driving : 90도
const float D = 45;    // Diagonal Driving : 45도


float PulsetoDegree(int pulse) {
  return pulse * d_revolution / e_revolution;
}

int DegreetoPulse(float degree) {
  return degree * e_revolution / d_revolution;
}


void resetEncoder() {
  uint8_t result = node.writeSingleRegister(0x0122, 1);
  if (result == node.ku8MBSuccess) {
    Serial.println("절대 위치 리셋 성공!");
  } else {
    Serial.print("리셋 실패. 에러 코드: ");
    Serial.println(result);
  }
  delay(500);
}

void servo_init() {
  Serial2.begin(9600, SERIAL_8N1, 23, 22);  // [RS485] RX=23, TX=22

  pinMode(pulsePin, OUTPUT);
  pinMode(signPin, OUTPUT);

  pinMode(MAX485_DE, OUTPUT);
  digitalWrite(MAX485_DE, 0);

  node.begin(MODBUS_ID, Serial2);
  node.preTransmission([]() {
    digitalWrite(MAX485_DE, 1);
  });
  node.postTransmission([]() {
    digitalWrite(MAX485_DE, 0);
  });
  node.writeSingleRegister(0x0018, 0x0000);

  // resetEncoder();
}


void Encoder() {
  uint8_t result = node.readHoldingRegisters(0x1018, 4);  // 0x1018 ~ 0x101B 읽기
  if (result != node.ku8MBSuccess) {
    Serial.println("Encoder(): 레지스터 읽기 실패");
    return;
  }

  uint32_t reg[4];
  for (int i = 0; i < 4; i++) {
    reg[i] = node.getResponseBuffer(i);
  }

  // uint64_t absPos = ((uint64_t)reg[3] << 48) | ((uint64_t)reg[2] << 32) | ((uint64_t)reg[1] << 16) | (uint64_t)reg[0];
  signedPos = (reg[1] << 16) | reg[0];

  char buffer[128];
  snprintf(
    buffer, sizeof(buffer),
    "signedPos: %6d | 현재 위치 (deg): %6.2f",
    signedPos, PulsetoDegree(signedPos));
  Serial.println(buffer);
}

void movetoposition(float target_deg) {
  int32_t target_pulse = DegreetoPulse(target_deg);
  int32_t scaled_pulse = (target_pulse - signedPos) / 32;  // 움직여야 하는 양 = 원하는 위치(deg) - 현재 위치 (deg)

  digitalWrite(signPin, scaled_pulse > 0);  // HIGH = 역방향(엔코더 +) 반시계방향

  char buffer[128];
  snprintf(buffer, sizeof(buffer),
           "signedPos: %6d | target_pulse: %6d | scaled_pulse: %6d",
           signedPos, target_pulse, scaled_pulse);
  Serial.println(buffer);

  for (int i = 0; i < abs(scaled_pulse); i++) {
    digitalWrite(pulsePin, HIGH);
    delayMicroseconds(250);  // 속도 조절 (짧을수록 빠름)
    digitalWrite(pulsePin, LOW);
    delayMicroseconds(250);
  }
}


void command(char mode) {
  switch (mode) {
    case 'o':
      Serial.println("Return Origin 시작");
      movetoposition(O);
      break;
    case 'c':
      Serial.println("Crab Driving 시작");
      movetoposition(C);
      break;
    case 'z':
      Serial.println("Zero Turn 시작");
      movetoposition(Z);
      break;
    case 'd':
      Serial.println("Diagonal Driving 시작");
      movetoposition(D);
      break;
    case 'e':
      Encoder();
      break;
    case 'p':
      resetEncoder();
      break;

    default:
      Serial.println("알 수 없는 명령입니다. 다시 입력해주세요.");
      break;
  }
  Serial.println();
}


void Normal() {
  Serial.println("각도를 입력하세요 : ");
  while (!Serial.available())
    ;

  float H = Serial.parseFloat();
  Serial.print("입력 각도 : ");
  Serial.println(H);
  movetoposition(H);
}
