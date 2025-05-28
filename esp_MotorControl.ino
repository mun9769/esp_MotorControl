// Purpose : Direction control by Serial input
// Pulse : FA-11 -> 4096, Encoder -> 131072
#include <WiFi.h>
#include <esp_chip_info.h>
#include "myServo.h"
#include "myCan.h"

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, 23, 22);  // [RS485통신 관련 포트] RX=23(DATA 받기), TX=22(명령 전송)

  can_init();
  servo_init();

  uint64_t chipid = ESP.getEfuseMac();

  Serial.printf("EFUSE MAC: %04X%08X\n",
                (uint16_t)(chipid >> 32), (uint32_t)chipid);
  if (chipid == 0xC0F44B3B015C)
    Serial.println("hawing");
  else if (chipid == 0x5C5F50B7B3F8)
    Serial.println("hahawing");
}

void loop() {
  if (onReceiveCANFrame() == true) {
    handleCanMessage();
  }
}
