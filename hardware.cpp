#include "hardware.h"
#include "sconv/sconv.hpp"
#include <ArduinoOTA.h>
#include <sstream>
ESP32_IRrecv ir;

TFT_eSprite dispBuf = TFT_eSprite(&M5.Lcd);



void initHardWare() {
  pinMode(10,OUTPUT);
  digitalWrite(10,LOW);//红色led

  uint8_t resetReason = rtc_get_reset_reason(0);
  if (resetReason == 5) {
    ESP_LOGI("init", "ESP32 reseted from deep sleep");
  } else {
    Wire.begin(32, 33);  //避免选择的开发板不是M5stickC而造成的io引脚冲突
    M5.begin(true, false, true);  //初始化屏幕和串口，跳过电源管理
    M5.Axp.begin();  //初始化电源管理(先初始化屏幕以避免花屏)
    M5.Axp.SetChargeCurrent(CURRENT_190MA);
  };
  M5.Lcd.setSwapBytes(true);
  dispBuf.createSprite(160, 80);
  dispBuf.setSwapBytes(true);
  pinMode(37, INPUT);
  pinMode(39, INPUT);

  WiFi.mode(WIFI_OFF);  //关闭WiFi
  disableMic();
  M5.Lcd.setRotation(1);
  M5.Lcd.loadHzk16();
  M5.Axp.ScreenBreath(11);
  M5.IMU.Init();
  if (M5.IMU.imuType == M5.IMU.IMU_MPU6886) {
    ESP_LOGI("init", "IMU type is MPU6886");
  } else {
    ESP_LOGI("init", "IMU type is SH200Q");
  };
  // rtc
  Wire1.beginTransmission(0x51);
  Wire1.write(0x01);
  Wire1.write(0x00);
  Wire1.endTransmission();
  delay(1);
  // imu
  Wire1.beginTransmission(0x68);
  Wire1.write(0x37);
  Wire1.write(0xd0);
  Wire1.endTransmission();
  delay(1);
  Wire1.beginTransmission(0x68);
  Wire1.write(0x38);
  Wire1.write(0x00);
  Wire1.endTransmission();
  delay(1);
  Wire1.beginTransmission(0x68);
  Wire1.write(0x58);
  Wire1.endTransmission();
  Wire1.requestFrom(0x68, 1);
  delay(1);

  // pmu
  uint32_t ReData = 0;
  Wire1.beginTransmission(0x34);
  Wire1.write(0x44);
  Wire1.endTransmission();
  Wire1.requestFrom(0x34, 4);
  for (int i = 0; i < 4; i++) {
    ReData <<= 8;
    ReData |= Wire1.read();
  };
  ESP_LOGI("init", "AXP192 IRQ state:%x", ReData);
  Wire1.beginTransmission(0x34);
  Wire1.write(0x31);
  Wire1.write(0x07);  //关机电压:3.3v
  //清除所有irq
  Wire1.write(0x44);
  Wire1.write(0xff);
  Wire1.write(0x45);
  Wire1.write(0xff);
  Wire1.write(0x46);
  Wire1.write(0xff);
  Wire1.write(0x47);
  Wire1.write(0xff);
  Wire1.endTransmission(true);
  
  rtc_gpio_deinit(GPIO_NUM_35);
  pinMode(GPIO_NUM_35, INPUT);
  ESP_LOGI("init","Interrupt state:%d",digitalRead(35));
  setCpuFrequencyMhz(80);
  ESP_LOGI("init", "CPU freq is now %d.", ESP.getCpuFreqMHz());
  digitalWrite(10,HIGH);
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
void deepSleep() {
  M5.Axp.SetSleep();
  Serial.println(
      esp_sleep_enable_ext0_wakeup(GPIO_NUM_35, 0));  //中断引脚被拉低时唤醒
  ESP_LOGI("hardware", "Start deep sleep");
  esp_deep_sleep_start();
}

float getPMUTemp() { return M5.Axp.GetTempInAXP192(); };
float getIMUTemp() {
  float temp = 0;
  M5.IMU.getTempData(&temp);
  return temp;
};

void textOut(string str, int16_t x, int16_t y, int8_t size_, uint32_t color, uint32_t bgColor) {
  dispBuf.setTextColor(color, bgColor);
  if (size_ != -1) dispBuf.setTextSize(size_);
  if (x != -1 && y != -1) dispBuf.setCursor(x, y, 2);  //默认第二个字体
  dispBuf.print(str.c_str());  //这个函数居然只能用const char*
};

queue<GBTextOnScreen*> GBTextQueue;

//因为不能向缓存里直接写入中文字符数据，因此先存在队列中，再用 textOutGB_Commit()输出
void textOutGB(const char *str, int16_t x, int16_t y, int8_t size_, uint32_t color,
               uint32_t bgColor) {
  if(GBTextQueue.size()>150){
    ESP_LOGW("hardware","Too many queued GB char!");
    return;
  }
  GBTextQueue.push(new GBTextOnScreen{
      .str = str,
      .x = x == -1 ? dispBuf.getCursorX() : x,
      .y = y == -1 ? dispBuf.getCursorY() : y,
      .size_ = size_ == -1 ? dispBuf.textsize : size_,
      .color = color,
      .bgColor = bgColor});
  //处理光标问题
  if (size_ != -1) dispBuf.setTextSize(size_);
  if (x != -1 && y != -1) dispBuf.setCursor(x, y, 2);
  if(str == NULL) return;
  char* tmp = const_cast<char*>(str);
  while(*tmp != '\0'){
    while(*tmp <= 0xA0){
      if(*tmp == '\0') return;
      dispBuf.setCursor(dispBuf.getCursorX()+8*dispBuf.textsize,dispBuf.getCursorY());
      tmp++;  
    }
    dispBuf.setCursor(dispBuf.getCursorX()+16*dispBuf.textsize,dispBuf.getCursorY());
    tmp+=2;
  };
};
void textOutU8(wstring str, int16_t x, int16_t y, int8_t size_, uint32_t color, uint32_t bgColor) {
  using namespace sconv;
  const wchar_t* ustr = str.c_str();
  uint8_t size = sconv_unicode_to_gbk((const wchar*)ustr, -1, NULL, 0);
  char *ansi_str = new char[size + 1];
  size = sconv_unicode_to_gbk((const wchar*)ustr, -1, ansi_str, size);
  ansi_str[size] = 0;
  textOutGB(ansi_str, x, y, size_, color, bgColor);
};
void textOutGB_Commit(){
  while (!GBTextQueue.empty()){
    GBTextOnScreen* &tmp = GBTextQueue.front();
    M5.Lcd.setTextColor(tmp->color, tmp->bgColor);
    M5.Lcd.setTextSize(tmp->size_);
    M5.Lcd.setCursor(tmp->x, tmp->y, 2);
    M5.Lcd.writeHzk(const_cast<char*>(tmp->str));
    GBTextQueue.pop();
    delete tmp;
  };
};

void rect(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint32_t color) {
  dispBuf.fillRect(x1, y1, x2 - x1, y2 - y1, color);
};

void textOutAligned(string str, uint8_t x, uint8_t y, uint8_t size_,
                    uint32_t color, uint8_t alignment) {
  dispBuf.setTextDatum(alignment);
  dispBuf.setTextColor(color);
  dispBuf.setTextSize(size_);
  dispBuf.drawString(str.c_str(), x, y, 2);
  dispBuf.setTextDatum(TL_DATUM);
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
  time.tm_wday = Rdate.WeekDay; //周一=1,周日=7..
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

bool doArduinoOTA(){
  if (WiFi.getMode() == WIFI_OFF){
    ESP_LOGI("OTA", "OTA:WiFi is off. Enabling...");
  };
  //不论如何打开WiFi
  WiFi.begin();
  dispBuf.fillRect(0, 0, dispBuf.width(), dispBuf.height(), TFT_BLACK);
  textOut("Connecting to WiFi..", 0, 0, 1, 0xffffff);
  dispBuf.pushSprite(0,0);
  while (WiFi.status() != WL_CONNECTED){
    getKey(KEY_MAIN, &mainKeyStatus);
    if (mainKeyStatus.keyPressed && !mainKeyStatus.keyPressedPrev){
      textOut("OTA canceled",0,15);
      ESP_LOGI("OTA","OTA canceled");
      dispBuf.pushSprite(0,0);
      WiFi.disconnect(true);
      delay(1000);
      return 0;
    };
    delay(200);
  };
  //wifi已连接
  bool haveError = false;
  ArduinoOTA.setHostname(((String)("M5StickC-Classtimer-"+ WiFi.macAddress())).c_str());
  ArduinoOTA.onStart([]{
    textOut("OTA Start",0,31);
    ESP_LOGI("OTA","OTA start");
    dispBuf.pushSprite(0,0);
  });
  ArduinoOTA.onProgress([](size_t progress, size_t total){
    dispBuf.fillRect(0,47,dispBuf.width(),63,TFT_BLACK);
    stringstream ss;
    ss.precision(2);
    ss.setf(ios::fixed);
    ss << "Progress:" << ((float)progress/total)*100 << "%";
    textOut(ss.str(),0,31);
    dispBuf.pushSprite(0,0);
  });

  ArduinoOTA.onError([&haveError](error_t error){
    if (error == OTA_AUTH_ERROR) ESP_LOGE("OTA","OTA: Auth Failed");
    else if (error == OTA_BEGIN_ERROR) ESP_LOGE("OTA","OTA: Begin Failed");
    else if (error == OTA_CONNECT_ERROR) ESP_LOGE("OTA","OTA: Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) ESP_LOGE("OTA","OTA: Receive Failed");
    else if (error == OTA_END_ERROR) ESP_LOGE("OTA","OTA: End Failed");
    textOut(("Error "+String(error)).c_str(),0,63);
    dispBuf.pushSprite(0,0);
    haveError = 1;
  });
  ArduinoOTA.onEnd([]{
    textOut("OTA end,rebooting..",0,47);
    dispBuf.pushSprite(0,0);
    ESP_LOGI("OTA","OTA end,rebooting..");
    delay(1000);
  });
  ArduinoOTA.begin();
  textOut("WiFi ok,waiting for OTA", 0, 15);
  dispBuf.pushSprite(0,0);
  while (!haveError){
    //
    ArduinoOTA.handle();
  };
  //函数不可能返回1,因为在更新成功后会重启
  return 0;
};
bool doWebOTA(){
  return 0;
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
  
  if (!keyStatus->keyPressedPrev && keyStatus->keyPressed)
    keyStatus->keyPressms = millis();
  if (keyStatus->keyPressedPrev && !keyStatus->keyPressed)
    keyStatus->keyReleasems = millis();
};
