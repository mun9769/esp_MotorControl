
#include <math.h>
using namespace std;

const uint64_t MAC_RL = 0x34C8DD32A7B0;
const uint64_t MAC_RR = 0x9C7CCDE342A8;
const uint64_t MAC_FL = 0x5C5F50B7B3F8;
const uint64_t MAC_FR = 0xC8604B3B015C;

void checkEspId() {
  uint64_t chipid = ESP.getEfuseMac();

  Serial.printf("EFUSE MAC: %04X%08X\n",
                (uint16_t)(chipid >> 32), (uint32_t)chipid);
  O = 0;
  D = 45;

  if (chipid == MAC_FL) {
    Z = 49.3;
    C = 90;
    onCalculateDegree = [](float joy_angle) -> float {
        return atan2(11, 11/tan(joy_angle)-9);
    };
  } else if (chipid == MAC_FR) {
    Z = -49.3;
    C = -90;
    onCalculateDegree = [](float joy_angle) -> float {
        return atan2(11, 11/tan(joy_angle)+9);
    };

  } else if (chipid == MAC_RL) {
    Z = -49.3;
    C = -90;
    onCalculateDegree = [](float joy_angle) -> float {
        return atan2(11, 11/tan(-joy_angle)-9);
    };

  } else if (chipid == MAC_RR) {
    Z = 49.3;
    C = 90;
    onCalculateDegree = [](float joy_angle) -> float {
        return atan2(11, 11/tan(-joy_angle)+9);
    };
  } else {
    Serial.println("없는 id입니다");
  }
}


