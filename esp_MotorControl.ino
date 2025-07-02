// Purpose : Direction control by Serial input
// Pulse : FA-11 -> 4096, Encoder -> 131072
#include <esp_chip_info.h>
#include "myUtil.h"
#include "myServo.h"
#include "myEsp.h"
#include "myCan.h"

void setup() {
  Serial.begin(115200);

  can_init();
  servo_init();
  checkEspId();
}

CANMessage msg;
void loop() {

  if (onReceiveCANFrame() == true) {
    int16_t deg = toInt16BE(rxMsg.data);
    char RxMode = rxMsg.data[3];

    command(RxMode, deg);
  }
}
