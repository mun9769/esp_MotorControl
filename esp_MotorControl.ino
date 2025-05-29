// Purpose : Direction control by Serial input
// Pulse : FA-11 -> 4096, Encoder -> 131072
#include <WiFi.h>
#include <esp_chip_info.h>
#include "myServo.h"
#include "myCan.h"

void setup() {
  Serial.begin(115200);

  can_init();
  servo_init();
  // uint64_t chipid = ESP.getEfuseMac();

  // Serial.printf("EFUSE MAC: %04X%08X\n",
  //               (uint16_t)(chipid >> 32), (uint32_t)chipid);
}

void loop() {
  if (onReceiveCANFrame() == true) {
    handleCanMessage();
  }
}
