
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

const float e_revolution = 131072;  // Encoder : 1바퀴당 Pulse (=2^17)
const float d_revolution = 360;

float CnttoDegree(uint32_t cnt) {
  return cnt * d_revolution / e_revolution;
}

uint32_t DegreetoCnt(float degree) {
  return degree * e_revolution / d_revolution;
}



void printModeDescription(char mode) {
  switch (mode) {
    case 'o': Serial.println("Return Origin 시작"); break;
    case 'c': Serial.println("Crab Driving 시작"); break;
    case 'z': Serial.println("Zero Turn 시작"); break;
    case 'd': Serial.println("Diagonal Driving 시작"); break;
    case 'e': Serial.println("Encoder() 호출"); break;
    case 'p': Serial.println("resetEncoder() 호출"); break;
    default: Serial.println("알 수 없는 명령입니다. 다시 입력해주세요."); break;
  }
}






