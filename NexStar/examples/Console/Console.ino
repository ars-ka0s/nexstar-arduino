#include <NexStar.h>

#define RXPIN 10
#define TXPIN 11
#define SELPIN 12

void printHex(byte val) {
  byte n = (val & 0xF0) >> 4;
  
  if (n < 10) {
    Serial.write('0' + n);
  } else {
    Serial.write('A' + n - 10);
  }

  n = val & 0x0F;
  
  if (n < 10) {
    Serial.write('0' + n);
  } else {
    Serial.write('A' + n - 10);
  }

  Serial.write(" ");
}

NexStar ns(TXPIN, RXPIN, SELPIN);
uint8_t currRate=5;

void setup() {
  Serial.begin(9600);
  while (!Serial);
}

void loop() {
  Serial.print("> ");
  
  while (!Serial.available());
  byte b = Serial.read();
  long p;
  uint16_t ver;
  
  switch (b) {
    case 'w':
      ns.elevationMC->jog(currRate, FORWARD);
      Serial.println("Up");
      break;
      
    case 's':
      ns.elevationMC->jog(currRate, REVERSE);
      Serial.println("Down");
      break;
      
    case 'd':
      ns.azimuthMC->jog(currRate, FORWARD);
      Serial.println("Right");
      break;
      
    case 'a':
      ns.azimuthMC->jog(currRate, REVERSE);
      Serial.println("Left");
      break;
      
    case 'x':
      ns.azimuthMC->jog(0, FORWARD);
      ns.elevationMC->jog(0, FORWARD);
      Serial.println("Stop");
      break;
      
    case '+':
      if (++currRate > 9) currRate = 9;
      Serial.print("Rate: ");
      Serial.println(currRate);
      break;
      
    case '-':
      if (--currRate < 1) currRate = 1;
      Serial.print("Rate: ");
      Serial.println(currRate);
      break;
      
    case 'z':
      Serial.print("Az: ");
      Serial.print(ns.azimuthMC->getPosition());
      Serial.print(" El: ");
      Serial.println(ns.elevationMC->getPosition());
      break;
      
    case 'q':
      p = Serial.parseInt();
      Serial.print("Move Az to ");
      Serial.println(p);
      ns.azimuthMC->gotoFast(p);
      break;

    case 'e':
      p = Serial.parseInt();
      Serial.print("Move El to ");
      Serial.println(p);
      ns.elevationMC->gotoFast(p);
      break;
    
    case 'Q':
      p = Serial.parseInt();
      Serial.print("Set Az to ");
      Serial.println(p);
      ns.azimuthMC->setPosition(p);
      break;

    case 'E':
      p = Serial.parseInt();
      Serial.print("Set El to ");
      Serial.println(p);
      ns.elevationMC->setPosition(p);
      break;

    case 'm':
      Serial.print("Moving: ");
      if (!ns.azimuthMC->isSlewing()) Serial.print("!");
      Serial.print("Az ");
      if (!ns.elevationMC->isSlewing()) Serial.print("!");
      Serial.println("El");
      break;

    case 'v':
      Serial.print("Az: ");
      ver = ns.azimuthMC->getVersion();
      printHex((ver & 0xFF00) >> 8);
      printHex(ver & 0xFF);

      Serial.print(" El: ");
      ver = ns.elevationMC->getVersion();
      printHex((ver & 0xFF00) >> 8);
      printHex(ver & 0xFF);
      break;

    case '?':
      Serial.println("w/s: up/down");
      Serial.println("a/d: left/right");
      Serial.println("x: stop");
      Serial.println("z: read current pos");
      Serial.println("+/-: change rate");
      Serial.println("q/e: send az/el to pos fast");
      Serial.println("Q/E: set az/el to pos");
      Serial.println("m: moving?");
      Serial.println("v: version?");
  }
}
