
#include <ACAN_ESP32.h>
CANMessage rxMsg;

static const uint32_t RX_STM32_TO_ESP32_ID = 0x18ff0101;  // STM32에서 오는 제어 명령 수신 ID (extended)

// ---------- 빅엔디안 변환 함수 ---------- //
// 4바이트 -> int32_t (빅엔디안)
static int32_t toInt32BE(const uint8_t* d) {
  return (int32_t)(((int32_t)d[0] << 24) | ((int32_t)d[1] << 16) | ((int32_t)d[2] << 8) | d[3]);
}

// 2바이트 -> int16_t (빅엔디안)
static int16_t toInt16BE(const uint8_t* d) {
  return (int16_t)(((int16_t)d[0] << 8) | d[1]);
}

// int32_t -> 4바이트 (빅엔디안)
static void to4ByteBE(uint8_t* d, int32_t v) {
  d[0] = (v >> 24) & 0xFF;
  d[1] = (v >> 16) & 0xFF;
  d[2] = (v >> 8) & 0xFF;
  d[3] = v & 0xFF;
}

// int16_t -> 2바이트 (빅엔디안)
static void to2ByteBE(uint8_t* d, int16_t v) {
  d[0] = (v >> 8) & 0xFF;
  d[1] = v & 0xFF;
}


void can_init() {
  // ----- CAN 초기화 (500kbps, extended ID 사용) ----- //
  ACAN_ESP32_Settings settings(500000UL);
  settings.mRxPin = GPIO_NUM_17;  // CAN RX
  settings.mTxPin = GPIO_NUM_16;  // CAN TX
  uint32_t ret = ACAN_ESP32::can.begin(settings);
  if (ret == 0) {
    Serial.println("CAN Configuration OK!");
  } else {
    Serial.print("CAN Configuration error: 0x");
    Serial.println(ret, HEX);
  }
  Serial.println("System Ready. Waiting for CAN frames...");
}



bool onReceiveCANFrame() {
  ACAN_ESP32::can.receive(rxMsg);

  // todo: 드라이버로 id 필터링 할것.
  if ((rxMsg.id == RX_STM32_TO_ESP32_ID) && (rxMsg.len == 4)) {
    return true;
  }
  return false;
}
char RxPrvMode;

void handleCanMessage() {
  int16_t RxStmDegree = toInt16BE(rxMsg.data);
  char RxMode = rxMsg.data[3];

  if (RxMode == 'r') {
    Serial.print("    Control Degree = ");
    Serial.println(RxStmDegree/10);
    RxPrvMode = 'r';
    return;
  }

  // if (RxPrvMode == RxMode)
  //   return;

  // if (RxPrvMode != RxMode) {
    command(RxMode);
    RxPrvMode = RxMode;
    return;
  // }
}


// CAN 프레임 송신 (디버깅 포함)
bool sendCANMessage(CANMessage& msg) {
  bool ok = ACAN_ESP32::can.tryToSend(msg);
  if (ok) {
    Serial.print(">>> Sent CAN Frame: ID=0x");
    Serial.print(msg.id, HEX);
    Serial.print(" (EXT), DLC=");
    Serial.print(msg.len);
    Serial.print(", Data=[ ");
    for (int i = 0; i < msg.len; i++) {
      Serial.print(msg.data[i], HEX);
      if (i < msg.len - 1) Serial.print(" ");
    }
    Serial.println(" ]");
  } else {
    Serial.print(">>> Failed to Send CAN Frame: ID=0x");
    Serial.println(msg.id, HEX);
  }
  return ok;
}
