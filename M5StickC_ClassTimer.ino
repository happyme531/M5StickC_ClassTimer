//选择当前的时间方案(以后会做到自动切换的)
//#define CLASS_TABLE_SATURDAY

#include "hardware.h"
#include "noisedetection.h"
#include "timeutil.h"
#include "ui.h"
UIClass ui;

uint32_t classTime[50]; 
classEvent_t classEventType[50];
uint8_t classCount;

keyStatus_t mainKeyStatus;
keyStatus_t subKeyStatus;
keyStatus_t pwrKeyStatus;

uint32_t screenOnTime = 0;  //上一次屏幕变量的时间
uint32_t lastRefresh = 0;
uint32_t powerOffCountDown = 0;  //自动关机倒计时,单位ms,0表示禁用
uint8_t curScreenBrightness = 11;

void setup() {
  initHardWare();
  uint8_t resetReason = rtc_get_reset_reason(0);
  if (resetReason == 5) {
    ESP_LOGI("init", "ESP32 reseted from deep sleep");
  }
  //校准时间
  textOut("Hold the main key to calibrate time..", 0, 0, 1, RED);
  delay(500);
  getKey(KEY_MAIN, &mainKeyStatus);
  if (mainKeyStatus.keyPressed) {
    struct tm time = getNTPTime();
    if (time.tm_hour != 0) {  //没有网络时调用获得的这个值是0
      ESP_LOGI("main", "NTP succeed");
      RTCSetTime(time);
    } else {
      ESP_LOGW("main", "NTP failed");
    }
  };
  struct tm time = RTCGetTime();
  char buf[30];
  strftime(buf, sizeof(buf), "%Y/%m/%d %H:%M:%S", &time); 
  ESP_LOGI("main","Local time:%s",buf);

  if(time.tm_wday<=5){
    memcpy(classTime,classTime_1_5,sizeof(classTime_1_5));
    memcpy(classEventType,classEventType_1_5,sizeof(classEventType_1_5));
    ESP_LOGI("init","Event count is %d and %d",getArrayLength(classTime_1_5),getArrayLength(classEventType_1_5));
    classCount=getArrayLength(classTime_1_5);
  }else if (time.tm_wday == 6){
    memcpy(classTime,classTime_6,sizeof(classTime_6));
    memcpy(classEventType,classEventType_6,sizeof(classEventType_6));
    classCount=getArrayLength(classTime_6);
  }else{
    memcpy(classTime,classTime_7,sizeof(classTime_7));
    memcpy(classEventType,classEventType_7,sizeof(classEventType_7));
    classCount=getArrayLength(classTime_7);  
  };
  screenOnTime = millis();
  ui.setPage(0);
};

void loop() {
  //读取按键状态
  getKey(KEY_MAIN, &mainKeyStatus);
  getKey(KEY_SUB, &subKeyStatus);
  getKey(KEY_POWER, &pwrKeyStatus);
 

  //切换页面
  if (subKeyStatus.keyPressed == 1 && subKeyStatus.keyPressedPrev == 0) {
    if (ui.page < ui.maxPage) {
      ui.setPage(ui.page + 1);
    } else {
      ui.setPage(0);
    };
  };

  if (lastRefresh + ui.refreshInterval <= millis()) {
    lastRefresh = millis();
    ui.refresh();
  };

  if (pwrKeyStatus.keyPressed) {
     powerOff();
    //deepSleep();
  };

  if (mainKeyStatus.keyPressed == 1 || M5.Axp.GetVinVoltage() > 4.0) {  //按下主按钮或正在充电
    screenOnTime = millis();           //点亮屏幕
  };

  // 20秒之后让屏幕变暗
  if (screenOnTime + 12000 < millis()) {
    if (curScreenBrightness != 8) {
      M5.Axp.ScreenBreath(8);
      curScreenBrightness = 8;
      ESP_LOGV("main", "Screen dim");
    };
  } else {
    if (curScreenBrightness != 11) {
      M5.Axp.ScreenBreath(11);
      curScreenBrightness = 11;
      ESP_LOGV("main", "Screen dim reset");
    };
  };

  if (getTotalAcceleration() > 16) {
    screenOnTime = millis();
  };


  ESP_LOGI("main","int pin: %d",digitalRead(35));

  if (ui.allowLightSleep) {
    esp_sleep_enable_timer_wakeup(ui.refreshInterval * 1000);
    esp_light_sleep_start();  //刷新屏幕之后进入睡眠模式（确实能省不少电）
  } else {
    delay(ui.refreshInterval);
  };
};
