
#include <ACAN_ESP32.h>
CANMessage rxMsg;

static const uint32_t RX_STM32_TO_ESP32_ID = 0x18ff0101;  // STM32에서 오는 제어 명령 수신 ID (extended)
static const uint32_t can_mask = 0x1FFFFFFF;

void can_init() {
  // ----- CAN 초기화 (500kbps, extended ID 사용) ----- //
  ACAN_ESP32_Settings settings(500000UL);
  settings.mRxPin = GPIO_NUM_17;  // CAN RX
  settings.mTxPin = GPIO_NUM_16;  // CAN TX

  const ACAN_ESP32_Filter filter = ACAN_ESP32_Filter::singleExtendedFilter(ACAN_ESP32_Filter::data, RX_STM32_TO_ESP32_ID, 0x7FF);
  uint32_t errorCode = ACAN_ESP32::can.begin(settings, filter);
  if (errorCode == 0) {
    Serial.println("CAN Configuration OK!");
  } else {
    Serial.print("CAN Configuration error: 0x");
    Serial.println(errorCode, HEX);
  }
  Serial.println("System Ready. Waiting for CAN frames...");
}


bool onReceiveCANFrame() {
  bool received = ACAN_ESP32::can.receive(rxMsg);

  if (received == false)
    return false;

  if (rxMsg.id != RX_STM32_TO_ESP32_ID) {
    Serial.println("RX_STM32_TO_ESP32_ID가 아닌 메세지 옴");
  }
  // todo: 드라이버로 id 필터링 할것.
  if (rxMsg.id == RX_STM32_TO_ESP32_ID && rxMsg.len == 4) {
    return true;
  }
  return false;
}


// CAN 프레임 송신 (디버깅 포함)
bool sendCANMessage(CANMessage& msg) {
  bool ok = ACAN_ESP32::can.tryToSend(msg);
  if (ok) {
  } else {
    Serial.print(">>> Failed to Send CAN Frame: ID=0x");
    Serial.println(msg.id, HEX);
  }
  return ok;
}
