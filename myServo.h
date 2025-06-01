#include <ModbusMaster.h>
#include <algorithm>

ModbusMaster node;

#define MAX485_DE 4
#define MODBUS_ID 1

const int pulsePin = 26;  // 노란색
const int signPin = 25;   // 빨간색

int32_t current_cnt = 0;  // 모터의 현재 펄스(=위치)

// 모드별 각도 설정
float O = 0;
float Z = 0;  // Zero Trun : arctan(1100/950)
float C = 0;  // Crab Driving : 90도
float D = 0;  // Diagonal Driving : 45도

void setWheelInitialAngle(uint64_t chipid) {
  Serial.printf("EFUSE MAC: %04X%08X\n",
                (uint16_t)(chipid >> 32), (uint32_t)chipid);

  if (chipid == 0x00) {
    Z = 49.3;
    C = 90;
    D = 45;
  } else if (chipid == 0x01) {
    Z = 0;
    C = 0;
    D = 0;
  } else if (chipid == 0x02) {
    Z = 0;
    C = 0;
    D = 0;
  } else if (chipid == 0x03) {
    Z = 0;
    C = 0;
    D = 0;
  } else {
    Serial.println("없는 id입니다");
  }
}

void servo_init() {
  Serial2.begin(9600, SERIAL_8N1, 23, 22);  // [RS485] RX=23, TX=22
  uint64_t chipid = ESP.getEfuseMac();
  setWheelInitialAngle(chipid);

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
  reg[1] = node.getResponseBuffer(1);
  reg[0] = node.getResponseBuffer(0);

  current_cnt = (reg[1] << 16) | reg[0];

  char buffer[128];
  snprintf(
    buffer, sizeof(buffer),
    "current_cnt: %6d | 현재 위치 (deg): %6.2f",
    current_cnt, CnttoDegree(current_cnt));
  Serial.println(buffer);
}

void resetEncoder() {
  uint8_t result = node.writeSingleRegister(0x0122, 1);
  if (result == node.ku8MBSuccess) {
    Serial.println("절대 위치 리셋 성공!");
    current_cnt = 0;
  } else {
    Serial.print("리셋 실패. 에러 코드: ");
    Serial.println(result);
  }
  delay(500);
}


void movetoposition(float target_deg, int microDelay = 1000) {
  int32_t target_cnt = DegreetoCnt(target_deg);
  int32_t delta_cnt = target_cnt - current_cnt;
  if (abs(delta_cnt) > e_revolution / 2) {
    if (delta_cnt > 0) delta_cnt -= e_revolution;
    else delta_cnt += e_revolution;
  }
  int32_t scaled_pulse = 20 * delta_cnt / 32;

  digitalWrite(signPin, scaled_pulse > 0);  // HIGH = 역방향(엔코더 +) 반시계방향

  char buffer[128];
  snprintf(buffer, sizeof(buffer),
           "current_cnt: %6d | target_cnt: %6d | scaled_pulse: %6d | 델타각도: %4d",
           current_cnt, target_cnt, scaled_pulse, CnttoDegree(delta_cnt));
  Serial.println(buffer);

  for (int i = 0; i < abs(scaled_pulse); i++) {
    digitalWrite(pulsePin, HIGH);
    delayMicroseconds(microDelay);  // 속도 조절 (짧을수록 빠름)
    digitalWrite(pulsePin, LOW);
    delayMicroseconds(microDelay);
  }

  //todo: 비동기적으로 current_cnt를 가져오기.
  current_cnt = target_cnt;
}

char RxPrvMode = 'o';
void controlJoystick(int16_t deg) {
  deg = max<int16_t>(deg, -30);
  deg = min<int16_t>(deg, 30);
  deg = (deg + 360) % 360;

  if (RxPrvMode == 'r') {
    Serial.print("Degree received from joystick = ");
    Serial.println(deg);
    movetoposition(deg);
  } else {
    Serial.println("원점으로 이동하는 중...");
    movetoposition(O);
  }
}

void command(char mode, int16_t deg) {
  //Encoder();
  switch (mode) {
    case 'r':
      controlJoystick(deg);
      break;
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
  RxPrvMode = mode;
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
