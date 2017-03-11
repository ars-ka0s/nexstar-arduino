#ifndef _NEXSTAR_H_
#define _NEXSTAR_H_

#include <Arduino.h>
#include <SoftwareSerial.h>

// 24-bit arguments are fairly common in this command set.
// Make it a little more obvious, even though they have to be 32-bit under the hood.
typedef int32_t int24;

// Direction enum used to specify travel directions and other positive/negative type arguments.
typedef enum {
  FORWARD = 0,
  POSITIVE = 0,
  REVERSE = 1,
  NEGATIVE = 1
} direction;

typedef struct {
  byte len;
  byte *bytes;
} reply;

// NexStar class represents the entire mount communication interface.
// The two contained MotorControllers are used for all user-facing operations.
class NexStar {
  private:
    byte _txPin, _rxPin, _flagPin;
    byte _srcID;
    SoftwareSerial *_ss;

    reply _sendCommand(byte destID, byte cmd, byte len, byte *data);
    reply _sendCommand(byte destID, byte cmd, int24 data);
        
  public:
    NexStar(byte txPin, byte rxPin, byte flagPin);
    
    class MotorController;
    
    MotorController *azimuthMC, *elevationMC;
};

// MotorController class represents each of the two axes on the mount.
// Not all possible commands are implemented.
class NexStar::MotorController {
  private:
    NexStar *_nexStar;
    byte _destID;
    
    int24 _clamp_int24(int24 x);
    int24 _bytes_to_int24(byte *b);
    
  public:
    MotorController(NexStar *nexStar, byte destID);
  
    int24 getPosition();
    void setPosition(int24 pos);
    
    void jog(uint8_t rate, direction dir);
    void gotoFast(int24 pos);
    void gotoSlow(int24 pos);

    bool isSlewing();
    
    void setCordwrapPosition(int24 pos);
    void enableCordwrap(bool enable);
    
    void setApproachDirection(direction dir);
    void setBacklashCompensation(uint8_t backlash, direction dir);
    
    uint16_t getVersion();
};

#endif
