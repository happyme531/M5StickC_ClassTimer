#ifndef HARDWARE_H
#define HARDWARE_H

#include "ESP32_IR_Remote.h"
#include <M5StickC.h>
#include <WiFi.h>
#include <inttypes.h>
#include <string>
#include <time.h>
using namespace std;

extern ESP32_IRrecv ir;

enum keyEvent_t { NONE, PRESS, RELEASE }; //按键动作

enum keyIndex_t { KEY_MAIN, KEY_SUB, KEY_POWER };

typedef struct keyStatus {
  uint32_t keyPressms; //按键开始被按下的时间
  keyEvent_t keyEvent;
  bool keyPressed;
  bool keyPressedPrev;
} keyStatus_t;

extern keyStatus_t mainKeyStatus;
extern keyStatus_t subKeyStatus;
extern keyStatus_t pwrKeyStatus;

void initHardWare();

void powerOff();

/*
 * 显示一串字符
 */
void textOut(string str, uint8_t x, uint8_t y, uint8_t size_, uint32_t color);

/*
  对齐方式:
  TL_DATUM = Top left
  TC_DATUM = Top centre
  TR_DATUM = Top right
  ML_DATUM = Middle left
  MC_DATUM = Middle centre
  MR_DATUM = Middle right
  BL_DATUM = Bottom left
  BC_DATUM = Bottom centre
  BR_DATUM = Bottom right
*/

void textOutAligned(string str, uint8_t x, uint8_t y, uint8_t size_,
                    uint32_t color, uint8_t alignment);

void rect(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint32_t color);

struct tm RTCGetTime();

void RTCSetTime(struct tm time);

void getKey(keyIndex_t keyIndex, keyStatus_t *keyStatus);

float getTotalAcceleration();

template <class T> uint8_t getArrayLength(T &array) {
  return (sizeof(array) / sizeof(array[0]));
};

#endif
