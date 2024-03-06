#include "NexStar.h"

NexStar::NexStar(byte txPin, byte rxPin, byte flagPin) {
  _txPin = txPin;
  _rxPin = rxPin;
  _flagPin = flagPin;
  
  _srcID = 0x0D;
  
  azimuthMC = new MotorController(this, 0x10);
  elevationMC = new MotorController(this, 0x11);

  _ss = new SoftwareSerial(rxPin, txPin);
  _ss->begin(19200);
  pinMode(_txPin, OUTPUT);
  digitalWrite(_txPin, HIGH);
  pinMode(_rxPin, INPUT_PULLUP);
  pinMode(_flagPin, INPUT_PULLUP);

  return;
}

reply NexStar::_sendCommand(byte destID, byte cmd, byte len, byte *data) {
  // Tristate flag pin, so turn driver on and drive low.  Wait before sending data.
  pinMode(_flagPin, OUTPUT);
  digitalWrite(_flagPin, LOW);
  delay(1);

  // Checksum: ignore prefix byte, sum remainder of packet, take 2s complement of low byte of sum.
  unsigned char sum = 0x03 + len + _srcID + destID + cmd;

  // Packet format: 0x3B len srcID destID cmd data[0]... sum
  // Length covers srcID, destID, and cmd plus data bytes.
  _ss->write(0x3B);
  _ss->write((unsigned char)(0x03 + len));
  _ss->write(_srcID);
  _ss->write(destID);
  _ss->write(cmd);
  for (byte i = 0; i < len; i++) {
    _ss->write(data[i]);
    sum += data[i];
  }
  _ss->write((unsigned char)((~sum)+1));

  // Tristate flag pin, so turn driver off a little bit after data completes.
  delay(1);
  pinMode(_flagPin, INPUT_PULLUP);

  static byte buff[5];
  byte b = 0, i = 0;
  byte exlen = 10;
  sum = 0;

  reply r;
  r.len = 0;
  r.bytes = 0;

  unsigned long timeout = millis() + TIMEOUT;

  // Now read the reply from the controller.  Return null pointer on error.
  while (1) {
    if (millis() > timeout) return r;
    while (_ss->available() < 1){
      if (millis() > timeout) return r;
    };
    b = _ss->read();
    if ((i != 0) && (i != exlen)) sum += b;

    switch(i) {
      case 0:
        if (b != 0x3B) return r;
        break;
      case 1:
        exlen = b + 2;
        break;
      case 2:
        if (b != destID) return r;
        break;
      case 3:
        if (b != _srcID) return r;
        break;
      case 4:
        if (b != cmd) return r;
        break;
      default:
        if (i < 10) buff[i-5] = b;
        break;
    }

    if (i == exlen) {
      if (b == (byte)((~sum)+1)) {
        if (exlen > 10) {
          r.len = 5;
        } else {
          r.len = exlen - 5;
        }
        r.bytes = buff;
      }
      
      return r;
    }

    i++;
  }
}

// AVR is little-endian, so for sending as bytes an int has to be reversed.
reply NexStar::_sendCommand(byte destID, byte cmd, int24 data) {
  int24 d = __builtin_bswap32(data);
  return _sendCommand(destID, cmd, 3, (byte*)(&d)+1);
}

NexStar::MotorController::MotorController(NexStar *nexStar, byte destID) {
  _nexStar = nexStar;
  _destID = destID;
}

// Confines values to 24-bit range.
int24 NexStar::MotorController::_clamp_int24(int24 x) {
  static const int32_t POSLIM = 0x007FFFFF;
  static const int32_t NEGLIM = 0xFF800000;

  if (x > POSLIM) return POSLIM;
  if (x < NEGLIM) return NEGLIM;
  return x;
}

// Converts 3 bytes from array into a 32-bit integer with 24-bit range.
int24 NexStar::MotorController::_bytes_to_int24(byte* b) {
  int32_t ret = b[0];
  ret = (ret << 8) + b[1];
  ret = (ret << 8) + b[2];
  ret = ret << 8;
  ret = ret >> 8;

  return ret;
}
  
int24 NexStar::MotorController::getPosition() {
  reply res = _nexStar->_sendCommand(_destID, 0x01, 0, 0);
  
  if (res.len != 3) {
    return -1;
  } else {
    return _bytes_to_int24(res.bytes);
  }
}

void NexStar::MotorController::setPosition(int24 pos) {
  _nexStar->_sendCommand(_destID, 0x04, _clamp_int24(pos));
}

// Valid jog rates are 0 to 9.
void NexStar::MotorController::jog(uint8_t rate, direction dir) {
  if (rate > 9) rate = 9;
  
  switch (dir) {
    case FORWARD:
    default:
      _nexStar->_sendCommand(_destID, 0x24, 1, (byte*)(&rate));
      break;
    case REVERSE:
      _nexStar->_sendCommand(_destID, 0x25, 1, (byte*)(&rate));
      break;
  }  
}

void NexStar::MotorController::gotoFast(int24 pos) {
  _nexStar->_sendCommand(_destID, 0x02, _clamp_int24(pos));
}

void NexStar::MotorController::gotoSlow(int24 pos) {
  _nexStar->_sendCommand(_destID, 0x17, _clamp_int24(pos));
}

bool NexStar::MotorController::isSlewing() {
  reply res = _nexStar->_sendCommand(_destID, 0x13, 0, 0);
  
  if (res.len != 1) {
    return true;
  } else {
    return (res.bytes[0] == 0x00);
  }
}

void NexStar::MotorController::setCordwrapPosition(int24 pos) {
  _nexStar->_sendCommand(_destID, 0x3a, _clamp_int24(pos));  
}

void NexStar::MotorController::enableCordwrap(bool enable) {
  if (enable) {
    _nexStar->_sendCommand(_destID, 0x38, 0, 0);
  } else {
    _nexStar->_sendCommand(_destID, 0x39, 0, 0);
  }
}

void NexStar::MotorController::setApproachDirection(direction dir) {
  _nexStar->_sendCommand(_destID, 0xFD, 1, (byte*)&dir);  
}

// Valid backlash compensation values are 0-99.
void NexStar::MotorController::setBacklashCompensation(uint8_t backlash, direction dir) {
  if (backlash > 99) backlash = 99;
  
  switch (dir) {
    case FORWARD:
    default:
      _nexStar->_sendCommand(_destID, 0x10, 1, (byte*)(&backlash));
      break;
    case REVERSE:
      _nexStar->_sendCommand(_destID, 0x11, 1, (byte*)(&backlash));
      break;
  }
}

// Returns two bytes. Example: 0515 = version 05.15.
// Good function to check initial connectivity.
uint16_t NexStar::MotorController::getVersion() {
  reply ret = _nexStar->_sendCommand(_destID, 0xFE, 0, 0);
  if (ret.len != 2) {
    return -1;
  } else {
    return (ret.bytes[0]<<8)+ret.bytes[1];  
  }
}

