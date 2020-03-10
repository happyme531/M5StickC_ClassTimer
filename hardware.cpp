#include "hardware.h"

ESP32_IRrecv ir;

void initHardWare() {
  M5.begin(true, false, true);  //初始化屏幕和串口，跳过电源管理
  M5.Axp.begin();  //初始化电源管理(先初始化屏幕以避免花屏)
  ESP_LOGI("init", "CPU freq is initially %d.", ESP.getCpuFreqMHz());
  esp_pm_config_esp32_t pwrCfg = {
      .max_cpu_freq = RTC_CPU_FREQ_XTAL,
      .max_freq_mhz = 80,
      .min_cpu_freq = RTC_CPU_FREQ_XTAL,
      .min_freq_mhz = 0,
      .light_sleep_enable = true,
  };
  if (esp_pm_configure(&pwrCfg) == ESP_ERR_NOT_SUPPORTED) {
    ESP_LOGW("init", "Failed to change freq.");
  } else {
    ESP_LOGI("init", "CPU freq is now %d.", ESP.getCpuFreqMHz());
  };

  WiFi.mode(WIFI_OFF);  //关闭WiFi

  disableMic();
  M5.Lcd.setRotation(1);
  M5.Lcd.loadHzk16();
  M5.Axp.ScreenBreath(11);
  M5.IMU.Init();

  Wire1.beginTransmission(0x34);
  Wire1.write(0x31);
  Wire1.write(0x07);  //关机电压:3.3v
  Wire1.endTransmission();
};

void enableMic() {
  Wire1.beginTransmission(0x34);
  Wire1.write(0x90);
  Wire1.write(0x02);  // gpio0 setting
  Wire1.endTransmission();
};

void disableMic() {
  Wire1.beginTransmission(0x34);
  Wire1.write(0x90);
  Wire1.write(0x06);  //关闭麦克风电源
  Wire1.endTransmission();
};

void powerOff() {
  Wire1.beginTransmission(0x34);
  Wire1.requestFrom(0x32, 1);
  uint8_t buf = Wire1.read();
  Wire1.write(0x32);
  Wire1.write(buf | 0x80);  //关机
  Wire1.endTransmission();
};

float getPMUTemp() { return M5.Axp.GetTempInAXP192(); };
float getIMUTemp() {
  float temp = 0;
  M5.IMU.getTempData(&temp);
  return temp;
};

void textOut(string str, int16_t x , int16_t y , int8_t size_ ,
             uint32_t color , uint32_t bgColor) {
  M5.Lcd.setTextColor(color,bgColor);
  if (size_ != -1) M5.Lcd.setTextSize(size_);
  if (x != -1 && y != -1) M5.Lcd.setCursor(x, y, 2);  //默认第二个字体
  M5.Lcd.printf(str.c_str());  //这个函数居然只能用const char*
};

void textOutZh(char *str, int16_t x, int16_t y, int8_t size_,
               uint32_t color,uint32_t bgColor) {  //这个函数用来输出中文
  M5.Lcd.setTextColor(color,bgColor);
  if (size_ != -1) M5.Lcd.setTextSize(size_);
  if (x != -1 && y != -1) M5.Lcd.setCursor(x, y, 2); 
  M5.Lcd.writeHzk(str);
}

void rect(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint32_t color) {
  M5.Lcd.fillRect(x1, y1, x2 - x1, y2 - y1, color);
};

void textOutAligned(string str, uint8_t x, uint8_t y, uint8_t size_,
                    uint32_t color, uint8_t alignment) {
  M5.Lcd.setTextDatum(alignment);
  M5.Lcd.setTextColor(color);
  M5.Lcd.setTextSize(size_);
  M5.Lcd.drawString(str.c_str(), x, y, 2);
  M5.Lcd.setTextDatum(TL_DATUM);
};

struct tm RTCGetTime() {
  struct tm time;
  RTC_TimeTypeDef Rtime;
  RTC_DateTypeDef Rdate;  //这个date中的year是实际年份
  M5.Rtc.GetTime(&Rtime);
  M5.Rtc.GetData(&Rdate);  //怀疑应该是GetDate
  time.tm_sec = Rtime.Seconds;
  time.tm_min = Rtime.Minutes;
  time.tm_hour = Rtime.Hours;
  time.tm_mday = Rdate.Date;
  time.tm_mon = Rdate.Month;
  time.tm_year = Rdate.Year - 1900;  // struct tm里的year是实际年份减去1900 (坑)
  time.tm_wday = Rdate.WeekDay;
  return time;
};

/*
struct tm RTCGetTime() {
  struct tm time;
  RTC_TimeTypeDef Rtime;
  RTC_DateTypeDef Rdate; //这个date中的year是实际年份
  M5.Rtc.GetTime(&Rtime);
  M5.Rtc.GetData(&Rdate); //怀疑应该是GetDate
  time.tm_sec = 10;
  time.tm_min = 10;
  time.tm_hour = 10;
  time.tm_mday = Rdate.Date;
  time.tm_mon = Rdate.Month;
  time.tm_year = Rdate.Year - 1900; // struct tm里的year是实际年份减去1900 (坑)
  time.tm_wday = Rdate.WeekDay;
  return time;
}; */
void RTCSetTime(struct tm time) {
  RTC_TimeTypeDef Rtime;
  RTC_DateTypeDef Rdate;
  Rtime.Seconds = time.tm_sec;
  Rtime.Minutes = time.tm_min;
  Rtime.Hours = time.tm_hour;
  Rdate.Date = time.tm_mday;
  Rdate.Month = time.tm_mon;
  Rdate.Year = time.tm_year + 1900;
  Rdate.WeekDay = time.tm_wday;
  M5.Rtc.SetTime(&Rtime);
  M5.Rtc.SetData(&Rdate);
};

float getTotalAcceleration() {
  float accX = 0.0F;
  float accY = 0.0F;
  float accZ = 0.0F;
  M5.IMU.getAccelData(&accX, &accY, &accZ);  //这个函数的加速度单位是G
  accX *= 9.8;
  accY *= 9.8;
  accZ *= 9.8;

  return sqrt((float)(accX * accX + accY * accY + accZ * accZ));
};

void getKey(keyIndex_t keyIndex, keyStatus_t *keyStatus) {
  keyStatus->keyPressedPrev = keyStatus->keyPressed;
  switch (keyIndex) {
    case KEY_MAIN:
      keyStatus->keyPressed = !digitalRead(37);
      break;
    case KEY_SUB:
      keyStatus->keyPressed = !digitalRead(39);
      break;
    case KEY_POWER:
      keyStatus->keyPressed =
          M5.Axp.GetBtnPress() == 1 ? true : false;  //暂时忽略 长按返回2
      break;
  };
  if (!keyStatus->keyPressedPrev && keyStatus->keyPressed) keyStatus->keyPressms = millis();
  if (keyStatus->keyPressedPrev && !keyStatus->keyPressed) keyStatus->keyReleasems = millis();
};
