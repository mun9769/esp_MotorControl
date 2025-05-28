#include <ModbusMaster.h>

ModbusMaster node;

#define MAX485_DE 4
#define MODBUS_ID 1

// Pin Setting
const int pulsePin = 26;  // 노란색
const int signPin = 25;   // 빨간색

// Parameter
int64_t cur_pulse = 0;
int64_t signedPos = 0;
const float e_revolution = 131072;  // Encoder : 1바퀴당 Pulse
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

void preTransmission() {
  digitalWrite(MAX485_DE, 1);
}

void postTransmission() {
  digitalWrite(MAX485_DE, 0);
}

void servo_init() {
  pinMode(pulsePin, OUTPUT);
  pinMode(signPin, OUTPUT);

  pinMode(MAX485_DE, OUTPUT);
  digitalWrite(MAX485_DE, 0);

  node.begin(MODBUS_ID, Serial2);
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);
  node.writeSingleRegister(0x0018, 0x0000);
  resetEncoder();
}


void Encoder() {
  uint8_t result;
  uint16_t reg[4];

  // 0x1018 ~ 0x101B 읽기
  result = node.readHoldingRegisters(0x1018, 4);

  if (result == node.ku8MBSuccess) {
    for (int i = 0; i < 4; i++) {
      reg[i] = node.getResponseBuffer(i);
    }

    // 64비트 값 조합
    uint64_t absPos = ((uint64_t)reg[3] << 48) | ((uint64_t)reg[2] << 32) | ((uint64_t)reg[1] << 16) | (uint64_t)reg[0];
    signedPos = (int64_t)absPos;  // 현재 절대엔코더 값
  }
}

void movetoposition(float target_deg) {
  float target_pulse = DegreetoPulse(target_deg);
  float delta_pulse = target_pulse - signedPos;  // 움직여야 하는 양 = 원하는 위치(deg) - 현재 위치 (deg)
  int scaled_pulse = delta_pulse / 32;           // 131072(Encoder)보다 4096(기본셋팅)는 32배 작기 때문에 스케일링

  if (scaled_pulse < 0)          // 방향 설정
    digitalWrite(signPin, LOW);  // LOW = 정방향(엔코더 -) 시계방향
  else
    digitalWrite(signPin, HIGH);  // HIGH = 역방향(엔코더 +) 반시계방향
  

  for (int i = 0; i < abs(scaled_pulse); i++) {
    digitalWrite(pulsePin, HIGH);
    delayMicroseconds(50);  // 속도 조절 (짧을수록 빠름)
    digitalWrite(pulsePin, LOW);
    delayMicroseconds(50);
  }
}
void calc_pulse(int scaled_pulse) {
  
}


void command(char mode) {
  Encoder();
  switch (mode) {
    case 'p':
      Serial.print("현재 위치 (pulse): ");
      Serial.println(signedPos);
      Serial.print("현재 위치 (deg): ");
      Serial.println(PulsetoDegree(signedPos));

      resetEncoder();
      break;
    case 'r':
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
    default:
      Serial.println("알 수 없는 명령입니다. 다시 입력해주세요.");
      break;
  }
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
