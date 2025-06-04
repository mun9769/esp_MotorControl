
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

  if (chipid == MAC_RL || chipid == MAC_FR) {
    Z = 49.3;
    C = 90;
  } else if (chipid == MAC_RR || chipid == MAC_FL) {
    Z = -49.3;
    C = -90;
  } else {
    Serial.println("없는 id입니다");
  }
}

