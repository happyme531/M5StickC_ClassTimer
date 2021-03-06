#include "ui.h"
#include <WiFiClient.h>
#include <string>
#include "Slot.h"
#include "camerareceive.h"
#include "hardware.h"
#include "lang_zh.h"
#include "microphone_fft.h"
#include "noisedetection.h"
#include "timeutil.h"
using namespace std;

HTU21D htu21d;
Slot slots[3];
int state;
bool initSucceed = 0;
WiFiClient client;
uint32_t lastTime;
uint16_t frameCount = 0;
uint8_t fps;
uint8_t menuSelection=0;
void UIClass::refresh() {
  if (page == 0) {
    /*
    第一页:时间等基本信息
   */
    if (pageNeedInit) {
      this->refreshInterval = 500;
      pageNeedInit = 0;
    };
    //dispBuf.fillScreen()只会绘制左半屏，bug?
    dispBuf.fillRect(0,0,dispBuf.width(),dispBuf.height(),TFT_BLACK);
    string tempStr;

    struct tm time;
    time = RTCGetTime();
    char buf[30];
    strftime(buf, sizeof(buf), "%g/%m/%d %H:%M:%S", &time);  //打印当前时间
    tempStr = buf;
    textOutGB(str_now, 0, 0, 1, 0xffffff);
    textOut(tempStr);

    //课程倒计时部分
    uint8_t eventNum = getNextEventNum(time);
    if (eventNum < classCount) {
      time = hmsDiff(todaySec2tm(classTime[eventNum]),time);  //计算当前时间与下一节课时间的差值
      strftime(buf, sizeof(buf), "%H:%M:%S", &time);
      tempStr = buf;
      textOutGB(str_nextEvent, 0, 15, 1, YELLOW);
      textOut(tempStr);  //打印距离下个事件的时间
      textOutGB(str_type, 0, 30, 1, YELLOW);
      textOutGB(eventName[classEventType[eventNum]], -1, -1, -1, YELLOW);

      //下节课程显示部分
      /* 
      uint8_t classNum = 0;  //今天已完成课程的数量
      for (int i = 0; i < eventNum; i++) {
        if (classEventType[i] == CLASS_BEGIN) classNum++;
      };
      ESP_LOGD("ui", "weekday %d,class %d", time.tm_wday, classNum);
      if (classNum < getArrayLength(classTable[0])) {
        textOutGB(str_nextClass, 0, 45);
        textOutGB((char*)":");
        textOutGB(className[classTable[time.tm_wday - 1][classNum]]);
      };
      */
    } else {
      textOutGB(str_noClassLeft, 0, 15, 1, GREEN);
    };

    if (time.tm_min < 1) {  //距离下一个下课事件1分钟内时允许进行校准
      textOut("Ready to calibrate", 0, 45, 1, WHITE);
      if (mainKeyStatus.keyPressed == 1) {
        struct tm timeNow = RTCGetTime();
        //查找下一个事件对应的时间设为系统时间
        //其实这个方法不够全面，因为只有在系统时间较慢时才有效
        //不过这个手表的时间一直比学校时间慢，所以用起来也没问题
        time = todaySec2tm(classTime[eventNum]);
        timeNow.tm_sec = time.tm_sec;
        timeNow.tm_min = time.tm_min;
        timeNow.tm_hour = time.tm_hour;
        RTCSetTime(timeNow);
      };
    };
    //在按住按钮5秒后显示消息，并在按住8秒后锁定时间的秒为0
    //可以用来调慢时间
    //用于弥补上面对时机制的不足
    if(millis()-mainKeyStatus.keyPressms>5000 && mainKeyStatus.keyPressed &&mainKeyStatus.keyPressedPrev){ 
      textOut("Press 8sec to freeze second",0,45,1,RED);
      if(millis()-mainKeyStatus.keyPressms>8000){
        struct tm timeNow = RTCGetTime();
        timeNow.tm_sec = 0;
        RTCSetTime(timeNow);
      };
    }; 
    //textOutU8(L"你好中国!",0,45,1,RED);
    float vbat = M5.Axp.GetBatVoltage();
    int16_t current = M5.Axp.GetBatCurrent();
    stringstream ss;
    ss.precision(2);
    ss.setf(ios::fixed);
    if (current > 0) {  //放电状态
      textOutGB(str_battery, 0, 65, 1, GREEN);
      //现在才知道可以在Arduino里使用iostream库，之前用加号连接字符串真是太蠢了
      ss << ":" << vbat << "V," << current << "mA";
      textOut(ss.str(), -1, -1, -1, GREEN);
    } else {
      textOutGB(str_battery, 0, 65, 1, RED);
      ss << ":" << vbat << "V," << current << "mA";
      textOut(ss.str(), -1, -1, -1, RED);
    };
    dispBuf.pushSprite(0,0);//同步显示
    textOutGB_Commit();

  } else if (page == 1) {  //第二页:大字显示时间
    if (pageNeedInit) {
      this->refreshInterval = 500;
      pageNeedInit = 0;
    };
    dispBuf.fillRect(0,0,dispBuf.width(),dispBuf.height(),TFT_BLACK);;
    dispBuf.fillRect(0,0,dispBuf.width(),dispBuf.height(),TFT_BLACK);;
    string tempStr;
    struct tm time;
    time = RTCGetTime();
    char buf[30];
    strftime(buf, sizeof(buf), "%H:%M:%S", &time);  //打印当前时间
    tempStr = buf;
    textOut(tempStr, 0, 0, 2, 0xffffff);
    uint8_t eventNum = getNextEventNum(time);
    if (eventNum < classCount) {
      time = hmsDiff(todaySec2tm(classTime[eventNum]),time);  //计算当前时间与下一节课时间的差值
      strftime(buf, sizeof(buf), "%H:%M:%S", &time);
      tempStr = buf;
      textOut(tempStr, 0, 40, 2, YELLOW);  //打印距离下个事件的时间
    } else {
      textOut("------", 0, 40, 2, GREEN);
    };
    dispBuf.pushSprite(0,0);
    // dispBuf.fillScreen(dispBuf.color565(0, 0, 255));
  } else if (page == 2) {  //第三页:农历与额外日期信息
    if (pageNeedInit) {
      this->refreshInterval = 1000;
      pageNeedInit = 0;
    };

    struct tm time;
    time = RTCGetTime();
    int daysInMonth=0;
    RTC_DateTypeDef lunarDate = getLunarDate(time,&daysInMonth);
    dispBuf.setCursor(0,0);
    dispBuf.setTextSize(1);
    dispBuf.setTextColor(WHITE);
    dispBuf.fillRect(0,0,dispBuf.width(),dispBuf.height(),TFT_BLACK);;
    dispBuf.printf("lc: %d/%d/%d (%d)",lunarDate.Year,lunarDate.Month,lunarDate.Date,daysInMonth);
    textOut("ref: ",0,15,1);
    textOutGB(str_tianGans[yearTianGan(time.tm_year+1900)]);
    textOutGB(str_diZhis[yearDiZhi(time.tm_year+1900)]);
    textOutGB(str_tianGans[monthTianGan(time.tm_year+1900,time.tm_mon+1)]);
    textOutGB(str_diZhis[monthDiZhi(time.tm_mon+1)]);
    textOutGB(str_tianGans[dayTianGan(time.tm_year+1900,time.tm_mon+1,time.tm_mday)]);
    textOutGB(str_diZhis[dayDiZhi(time.tm_year+1900,time.tm_mon+1,time.tm_mday)]);
    dispBuf.pushSprite(0,0);
    textOutGB_Commit();


  } else if (page == 3) {  //第四页:遥控器功能
    if (pageNeedInit) {
      this->refreshInterval = 500;
      pageNeedInit = 0;
    };
    dispBuf.fillRect(0,0,dispBuf.width(),dispBuf.height(),TFT_BLACK);;
    textOutAligned((string) "Canteen TV Controller", 80, 0, 1, WHITE, TC_DATUM);
    dispBuf.pushSprite(0,0);
    if (mainKeyStatus.keyPressed) {
      ir.ESP32_IRsendPIN(9, 0);
      ir.initSend();
      delay(1000);
      ir.sendIR((irCode), getArrayLength(irCode));
      delay(1000);
      ir.stopIR();
      // digitalWrite(9,HIGH);
      textOut((string) "IR sent", 2, 15, 1, BLUE);
      dispBuf.pushSprite(0,0);
      delay(1000);
    };

  } else if (page == 4) {  //第五页:噪音检测
    if (pageNeedInit) {
      enableMic();
      micFFTInit();
      setCpuFrequencyMhz(240);
      this->refreshInterval = 8;
      this->allowLightSleep = 0;
      pageNeedInit = 0;
    };
    for (int i = 0; i < 15; i++) {
      micFFTRun();
      delay(10);
    };
    screenOnTime = millis();

  } else if (page == 5) {  //第六页:传感器数据
    if (pageNeedInit) {
      micFFTDeinit();
      disableMic();
      setCpuFrequencyMhz(80);
      this->allowLightSleep = 1;
      this->refreshInterval = 400;
      pinMode(36, INPUT);  // 36是个gpi口，只能用来输入

      Wire.begin(0, 26); //同上，不能用gpio36来进行i2c通讯
      htu21d.begin(Wire);
      Wire.beginTransmission(0x40);
      if (Wire.endTransmission() == 0) {  //表明设备已连接
        initSucceed = 1;
      } else {
        initSucceed = 0;
      };

      pageNeedInit = 0;
    };
    dispBuf.fillRect(0,0,dispBuf.width(),dispBuf.height(),TFT_BLACK);;
    uint16_t adcVal = analogRead(36);
    stringstream ss;
    ss << "ADC:" << adcVal << "->" << (float)(3.3 * adcVal / 4096) << "V";
    textOut(ss.str(), 3, 0, -1);
    ss.str("");
    if (initSucceed) {
      textOut("HTU21D init OK", 0, 20, 1, GREEN);
      ss << "Temp:" << htu21d.readTemperature();
      textOut(ss.str(), 0, 40, 1, WHITE);
      ss.str("");
      ss << "Humi:" << htu21d.readHumidity();
      textOut(ss.str(), 0, 60, 1, WHITE);

    } else {
      textOut("HTU21D init ERROR", 20, 0, 1, GREEN);
    };
    dispBuf.pushSprite(0,0);

  } else if (page == 6) {  //第七页:老虎机小游戏

    enum SlotsState {
      SLOTS_INIT,
      SLOTS_START,
      SLOTS_STOP = SLOT_COUNT + 1,
      SLOTS_FLUSH
    };

    const int symbolIndices[] = {2, 4, 5, 0, 3, 4, 1, 5, 3};
    if (pageNeedInit) {
      this->refreshInterval = 20;
      dispBuf.fillRect(0,0,dispBuf.width(),dispBuf.height(),TFT_BLACK);
      M5.Lcd.fillScreen(TFT_BLACK);
      dispBuf.setSwapBytes(true);
      Slot::initShadow();
      Slot::setReel(symbolIndices, getArrayLength(symbolIndices));
      for (int i = 0; i < 3; i++) {
        slots[i].init(i, i * SLOT_COUNT);
        slots[i].draw();
      };
      state = SLOTS_INIT;
      pageNeedInit = 0;
    };

    unsigned long tick = millis();
    static unsigned long flushTick;
    static int flushCount;
    const int8_t FLUSH_DELAY = 120;
    const int8_t FLUSH_COUNT = 3;

    if (mainKeyStatus.keyPressed && !mainKeyStatus.keyPressedPrev &&
        mainKeyStatus.keyReleasems + 200 < tick) {
      if (state == SLOTS_INIT) {
        for (int i = 0; i < SLOT_COUNT; i++) {
          slots[i].start();
        }
        state++;
      } else if (state < SLOTS_STOP) {
        slots[state - 1].stop();
        state++;
      }
    }
    if (state == SLOTS_STOP) {
      int symbol = -1;
      bool stopAll = true;
      for (int i = 0; i < SLOT_COUNT; i++) {
        int n = slots[i].getSymbol();
        if (n == -1) {
          stopAll = false;
        } else {
          symbol = (n == symbol || symbol == -1) ? n : -2;
        }
      }
      if (stopAll) {
        if (symbol >= 0) {
          flushTick = tick;
          flushCount = 0;
          state = SLOTS_FLUSH;
        } else {
          state = SLOTS_INIT;
        }
      }
    }
    if (state == SLOTS_FLUSH) {
      if (tick >= flushTick + FLUSH_DELAY) {
        flushTick = tick;
        for (int i = 0; i < SLOT_COUNT; i++) {
          slots[i].flush((flushCount & 1) ? TFT_WHITE : TFT_BLUE);
        }
        if (++flushCount >= FLUSH_COUNT * 2) {
          state = SLOTS_INIT;
        }
      }
    }

    for (int i = 0; i < SLOT_COUNT; i++) {
      if (slots[i].update()) {
        slots[i].draw();
      }
    }
    
  } else if (page == 7) {
    if (pageNeedInit) {
      dispBuf.fillRect(0,0,dispBuf.width(),dispBuf.height(),TFT_BLACK);
      textOutAligned("Wifi camera receiver", 80, 0, 1, WHITE, TC_DATUM);
      textOut("Press main key to start", 0, 20, 0, WHITE);
      dispBuf.pushSprite(0,0);
      this->refreshInterval = 500;
      this->allowLightSleep = 0;
      if (mainKeyStatus.keyPressed) {
        WiFi.mode(WIFI_MODE_STA);
        pageNeedInit = 0;
      };
      return;
    };

    while (WiFi.status() != WL_CONNECTED) {
      WiFi.begin("ESP32 Camera", "");
      dispBuf.fillRect(0,0,dispBuf.width(),dispBuf.height(),TFT_BLACK);;
      textOut("Connecting...", 0, 0, 0, WHITE);
      dispBuf.pushSprite(0,0);
      ESP_LOGI("cam", "Wifi connecting..");
      frameCount = 0;
      lastTime = millis();
      delay(1000);
    };

    if (this->refreshInterval > 50) this->refreshInterval = 20;

    updateFrame(client);
    frameCount++;
    // fps计数器
    if (millis() - lastTime >= 1000) {
      fps = frameCount;
      frameCount = 0;
      lastTime = millis();
    };
    dispBuf.setCursor(0, 0);
    dispBuf.printf("FPS:%d", fps);
    //显示电池状态
    float vbat = M5.Axp.GetBatVoltage();
    int16_t current = M5.Axp.GetBatCurrent();
    if (current > 0) {  //放电状态
      textOutGB(str_battery, 0, 65, 1, GREEN);
      textOut((":" + String(vbat) + "V," + String(current) + "mA").c_str(), -1,
              -1, -1, GREEN);
    } else {
      textOutGB(str_battery, 0, 65, 1, RED);
      textOut((":" + String(vbat) + "V," + String(current) + "mA").c_str(), -1,
              -1, -1, RED);
    };
    dispBuf.pushSprite(0,0);
  }else if (page==8){ //网络相关配置
    if (pageNeedInit) {
      pageNeedInit=0;
      refreshInterval=300;
    };
    //绘制菜单
    if(menuSelection==0){
      textOutGB(str_espTouch,0,0,1,BLACK,YELLOW);
    }else{
      textOutGB(str_espTouch,0,0,1,YELLOW,BLACK);
    };
    if(menuSelection==1){
      textOutGB(str_timeCalibrate,0,15,1,BLACK,YELLOW);
    }else{
      textOutGB(str_timeCalibrate,0,15,1,YELLOW,BLACK);
    };
    if(menuSelection==2){
      textOutGB(str_arduinoOTA,0,31,1,BLACK,YELLOW);
    }else{
      textOutGB(str_arduinoOTA,0,31,1,YELLOW,BLACK);
    };
    if(menuSelection==3){
      textOutGB(str_webOTA_Station,0,47,1,BLACK,YELLOW);
    }else{
      textOutGB(str_webOTA_Station,0,47,1,YELLOW,BLACK);
    };
    if(menuSelection==4){
      textOutGB(str_webOTA_AP,0,63,1,BLACK,YELLOW);
    }else{
      textOutGB(str_webOTA_AP,0,63,1,YELLOW,BLACK);
    };
    dispBuf.fillRect(0,0,dispBuf.width(),dispBuf.height(),TFT_BLACK);
    dispBuf.pushSprite(0,0);
    textOutGB_Commit();
    //处理按键
    //短按

    if (mainKeyStatus.keyPressed && !mainKeyStatus.keyPressedPrev && millis()-mainKeyStatus.keyPressms<=2000){
      if(menuSelection < 4){
        menuSelection++;
      }else{
        menuSelection=0;
      };
    };

    //长按,开始处理
    if(mainKeyStatus.keyPressed && millis()-mainKeyStatus.keyPressms>2000){
      menuSelection--; //权宜之计
      if(menuSelection==0){ //smartconfig
        dispBuf.fillRect(0,0,dispBuf.width(),dispBuf.height(),TFT_BLACK);
        textOut("SmartConfig start..",0,0);
        ESP_LOGI("ui","SmartConfig start..");
        dispBuf.pushSprite(0,0);
        WiFi.beginSmartConfig();
        textOut("Waiting for SmartConfig",0,15);
        ESP_LOGI("ui","Waiting for SmartConfig");
        dispBuf.pushSprite(0,0);
        while (!WiFi.smartConfigDone()){
          delay(200);
          getKey(KEY_MAIN , &mainKeyStatus);
          if(mainKeyStatus.keyPressed && !mainKeyStatus.keyPressedPrev){
            textOut("SmartConfig canceled",0,31);
            ESP_LOGI("ui","SmartConfig canceled");
            dispBuf.pushSprite(0,0);
            WiFi.stopSmartConfig();
            delay(1000);
            return;
          };
        };
        textOut("Connecting to wifi",0,31);
        ESP_LOGI("ui","Smartconfig:Connecting to wifi");
        dispBuf.pushSprite(0,0);
        while (WiFi.status() != WL_CONNECTED){
          delay(200);
          getKey(KEY_MAIN , &mainKeyStatus);
          if(mainKeyStatus.keyPressed && !mainKeyStatus.keyPressedPrev){
            textOut("SmartConfig canceled",0,47);
            ESP_LOGI("ui","SmartConfig canceled");
            dispBuf.pushSprite(0,0);
            WiFi.disconnect(true,true);
            delay(1000);
            return;
          };
        };
        textOut("Connect succeed",0,31);
        ESP_LOGI("ui","Smartconfig:onnect succeed");
        dispBuf.pushSprite(0,0);
        WiFi.disconnect(true); //关闭wifi,此时已经储存了信息
        delay(1000);
      }else if(menuSelection==1){ //ntp对时
        struct tm time = getNTPTime();
        if (time.tm_hour != 0) {  //没有网络时调用获得的这个值是0
          ESP_LOGI("main", "NTP succeed");
          RTCSetTime(time);
        } else {
          ESP_LOGW("main", "NTP failed");
        };
      }else if(menuSelection == 2){ //ArduinoOTA
      doArduinoOTA();
      };
    };
  };
  return;
};

void UIClass::setPage(uint8_t page_) {
  // this->refreshInterval = this->refreshIntervals[page_];
  this->allowLightSleep = 1;
  this->pageNeedInit = 1;
  this->page = page_;
};
