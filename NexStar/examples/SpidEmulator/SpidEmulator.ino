// Emulates the SPID Rot2Prog AzEl rotator protocol.

#include <NexStar.h>

#define RXPIN 10
#define TXPIN 11
#define SELPIN 12

#define TICKS_PER_DEG 46603.378f

#define DEFAULT_RESOLUTION 10

NexStar ns(TXPIN, RXPIN, SELPIN);

void sendStatus() {
  float az = (float)ns.azimuthMC->getPosition() / TICKS_PER_DEG;
  float el = (float)ns.elevationMC->getPosition() / TICKS_PER_DEG;

  int h14 = (az + 360.0f) * 10.0f;
  int v14 = (el + 360.0f) * 10.0f;

  Serial.write(0x57);
  Serial.write((byte)(h14 / 1000));
  Serial.write((byte)(h14 / 100 % 10));
  Serial.write((byte)(h14 / 10 % 10));
  Serial.write((byte)(h14 % 10));
  Serial.write(DEFAULT_RESOLUTION);
  Serial.write((byte)(v14 / 1000));
  Serial.write((byte)(v14 / 100 % 10));
  Serial.write((byte)(v14 / 10 % 10));
  Serial.write((byte)(v14 % 10));
  Serial.write(DEFAULT_RESOLUTION);
  Serial.write(0x20);
  
  return;
}

void setup() {
  Serial.begin(600);
}

void loop() {
  static byte i = 0;
  static int h14 = 0, v14 = 0;
  static int ph = 0, pv = 0;
  static byte cmd;
    
  while (Serial.available() < 1);
  byte b = Serial.read();

  switch (i) {
    case 0:
      if (b != 0x57) return;
      break;
    case 1:
      h14 = (b - '0');
      break;
    case 2:
    case 3:
    case 4:
      h14 = h14 * 10 + (b - '0');
      break;
    case 5:
      ph = b;
      break;
    case 6:
      v14 = (b - '0');
      break;
    case 7:
    case 8:
    case 9:
      v14 = v14 * 10 + (b - '0');
      break;
    case 10:
      pv = b;
      break;
    case 11:
      cmd = b;
      break;
    case 12:
      if (b != 0x20) {
        i = 0;
        return;
      }
  }

  if (i == 12) {
    switch (cmd) {
      case 0x0F: // Stop
        ns.azimuthMC->jog(0, FORWARD);
        ns.elevationMC->jog(0, FORWARD);
      case 0x1F: // Status
        sendStatus();
        break;
      case 0x2F: // Set
        if (ph == 0) ph = DEFAULT_RESOLUTION;
        if (pv == 0) pv = DEFAULT_RESOLUTION;
        
        float az = (float)h14 / ph - 360.0f;
        float el = (float)v14 / pv - 360.0f;

        ns.azimuthMC->gotoFast(TICKS_PER_DEG * az);
        ns.elevationMC->gotoFast(TICKS_PER_DEG * el);
        break;
    }
    
    i = 0;
  } else {
    i++;
  }
}
