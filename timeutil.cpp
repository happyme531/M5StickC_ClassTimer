#include "timeutil.h"
#include "hardware.h"
#include <WiFi.h>
#include <ctime>

const char *wifiSSID_ = "bu~zhun~ceng~wang(2.4G)";
const char *wifiPassword_ = "zlq13834653953";
char* eventName[]={str_classReady,str_classOver,str_classBegin,str_lateForSchool,str_xieZuRequire};
struct tm getNTPTime() {
  if (WiFi.getMode() == WIFI_OFF) {
    //不要用ESP_LOGx,貌似是坏掉的
    Serial.println("NTP:WiFi is off. Enabling...");
    WiFi.begin(wifiSSID_, wifiPassword_);
  };
  M5.Lcd.fillScreen(BLACK);
  textOut((string) "Wifi on", 0, 0, 1, 0xffffff);
  delay(1300);
  keyStatus_t exitKey;
  exitKey.keyPressed = false;
  struct tm time;

  while (WiFi.status() != WL_CONNECTED) {
    getKey(KEY_MAIN, &exitKey);
    if (exitKey.keyPressed)
      goto end;
    delay(50);
  };

  configTime(8 * 60 * 60, 0, "ntp.ntsc.ac.cn");
end:
  textOut((string) "NTP end", 0, 10, 1, 0xffffff);
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  getLocalTime(&time);
  return time;
};

uint32_t tm2todaySec(struct tm time) {
  return time.tm_sec + time.tm_min * 60 + time.tm_hour * 60 * 60;
};

struct tm todaySec2tm(uint32_t sec) {
  struct tm time;
  time.tm_hour = sec / 3600;
  sec -= 3600 * (time.tm_hour);
  time.tm_min = sec / 60;
  sec -= 60 * (time.tm_min);
  time.tm_sec = sec;
  return time;
};

struct tm hmsDiff(struct tm time1, struct tm time2) {
  uint32_t sec1 = tm2todaySec(time1);
  uint32_t sec2 = tm2todaySec(time2);

  return todaySec2tm(sec1 < sec2 ? sec2 - sec1 : sec1 - sec2);
};

uint8_t getNextClassNum(struct tm timeNow) {

  uint32_t timeNowSec = tm2todaySec(timeNow);
  int i = 0;
  for (i = 0; timeNowSec > classTime[i]; i++)
    ;
  return i;
};
