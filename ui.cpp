#include "ui.h"
#include "Slot.h"
#include "camerareceive.h"
#include "hardware.h"
#include "noisedetection.h"
#include "string"
#include "timeutil.h"

using namespace std;

HTU21D htu21d;
Slot slots[3];
int state;
bool initSucceed = 0;
WiFiClient client;
uint32_t lastTime;
uint16_t frameCount=0;
uint8_t fps;
void UIClass::refresh()
{

    if (page == 0) {
        /*
        第一页:时间等基本信息
       */
        if (pageNeedInit) {
            this->refreshInterval = 500;
            pageNeedInit = 0;
        };
        M5.Lcd.fillScreen(BLACK);
        string tempStr;

        struct tm time;
        time = RTCGetTime();
        char buf[30];
        strftime(buf, sizeof(buf), "Now:%g-%m-%d %H:%M:%S", &time); //打印当前时间
        tempStr = buf;
        textOut(tempStr, 0, 0, 1, 0xffffff);
        uint8_t classNum = getNextClassNum(time);
        if (classNum < getArrayLength(classTime)) {
            time = hmsDiff(todaySec2tm(classTime[classNum]), time); //计算当前时间与下一节课时间的差值
            strftime(buf, sizeof(buf), "%H:%M:%S", &time);
            tempStr = (string) "Next event: " + buf;
            textOut(tempStr, 0, 15, 1, YELLOW); //打印距离下个事件的时间
            tempStr = (string) "Type: " + eventName[classEventType[classNum]];
            textOut(tempStr, 0, 30, 1, YELLOW);

        } else {
            textOut("No class left", 0, 15, 1, GREEN);
        };

        if (time.tm_min < 1) { //距离下一个下课事件1分钟内时允许进行校准
            textOut("Ready to calibrate", 0, 30, 1, WHITE);
            if (mainKeyStatus.keyPressed == 1) {
                struct tm timeNow = RTCGetTime();
                //查找下一个事件对应的时间设为系统时间
                //其实这个方法不够全面，因为只有在系统时间较慢时才有效
                //不过这个手表的时间一直比学校时间慢，所以用起来也没问题
                time = todaySec2tm(classTime[classNum]);

                timeNow.tm_sec = time.tm_sec;
                timeNow.tm_min = time.tm_min;
                timeNow.tm_hour = time.tm_hour;
                RTCSetTime(timeNow);
            };
        };
        textOut(("Temp: I:" + String(getIMUTemp()) + " P:" + String(getPMUTemp()) + "").c_str(), 0, 45, 1, WHITE);
        uint16_t vbat = M5.Axp.GetVbatData() * 1.1;
        uint8_t current = M5.Axp.GetIdischargeData() / 2;
        if (current > 0) { //放电状态
            textOut(("Vbat:" + String(vbat) + "mV,Bcur:" + String(current) + "mA").c_str(), 0, 65, 1, GREEN);
        } else {
            current = M5.Axp.GetIchargeData() / 2;
            textOut(("Vbat:" + String(vbat) + "mV,Bcur:" + String(current) + "mA").c_str(), 0, 65, 1, RED);
        };

    } else if (page == 2) { //第三页:遥控器功能
        if (pageNeedInit) {
            this->refreshInterval = 500;
            pageNeedInit = 0;
        };
        M5.Lcd.fillScreen(BLACK);
        textOutAligned((string) "Canteen TV Controller", 80, 0, 1, WHITE, TC_DATUM);
        if (mainKeyStatus.keyPressed) {
            ir.ESP32_IRsendPIN(9, 0);
            ir.initSend();
            delay(1000);
            ir.sendIR((irCode), getArrayLength(irCode));
            delay(1000);
            ir.stopIR();
            //digitalWrite(9,HIGH);
            textOut((string) "IR sent", 2, 15, 1, BLUE);

            delay(1000);
        };

    } else if (page == 1) { //第二页:大字显示时间
        if (pageNeedInit) {
            this->refreshInterval = 500;
            pageNeedInit = 0;
        };
        M5.Lcd.fillScreen(BLACK);
        string tempStr;
        struct tm time;
        time = RTCGetTime();
        char buf[30];
        strftime(buf, sizeof(buf), "%H:%M:%S", &time); //打印当前时间
        tempStr = buf;
        textOut(tempStr, 0, 0, 2, 0xffffff);
        uint8_t classNum = getNextClassNum(time);
        if (classNum < getArrayLength(classTime)) {
            time = hmsDiff(todaySec2tm(classTime[classNum]), time); //计算当前时间与下一节课时间的差值
            strftime(buf, sizeof(buf), "%H:%M:%S", &time);
            tempStr = buf;
            textOut(tempStr, 0, 40, 2, YELLOW); //打印距离下个事件的时间
        } else {
            textOut("------", 0, 40, 2, GREEN);
        };
        //M5.Lcd.fillScreen(M5.Lcd.color565(0, 0, 255));
    } else if (page == 3) { //第四页:噪音检测
        if (pageNeedInit) {
            this->refreshInterval = 40;
            pageNeedInit = 0;
        };
        M5.Lcd.fillScreen(BLACK);
        String tempStr;
        tempStr = String(getNoiseVal());
        textOut(tempStr.c_str(), 0, 0, 2, GREEN);

    } else if (page == 4) { //第五页:htu21d 温度传感器
        if (pageNeedInit) {
            this->refreshInterval = 500;
            pageNeedInit = 0;
        };
        if (pageNeedInit) {
            Wire.begin(36, 26, 100000);
            htu21d.begin(Wire);
            Wire.beginTransmission(0x40);
            if (Wire.endTransmission() == 0) { //表明设备已连接
                initSucceed = 1;
            } else {
                initSucceed = 0;
            };

            pageNeedInit = 0;
        };
        M5.Lcd.fillScreen(BLACK);
        if (initSucceed) {
            textOut("HTU21D init OK", 0, 0, 1, GREEN);
            String tempStr;

            tempStr = "Temp:" + String(htu21d.readTemperature());
            textOut(tempStr.c_str(), 0, 20, 1, WHITE);
            tempStr = "Humi:" + String(htu21d.readHumidity());
            textOut(tempStr.c_str(), 0, 40, 1, WHITE);

        } else {
            textOut("HTU21D init ERROR", 0, 0, 1, GREEN);
        };
    } else if (page == 5) { //第六页:老虎机小游戏

        enum SlotsState { SLOTS_INIT,
            SLOTS_START,
            SLOTS_STOP = SLOT_COUNT + 1,
            SLOTS_FLUSH };

        const int symbolIndices[] = { 2, 4, 5, 0, 3, 4, 1, 5, 3 };
        if (pageNeedInit) {
            this->refreshInterval = 20;
            M5.Lcd.fillScreen(TFT_BLACK);
            M5.Lcd.setSwapBytes(true);
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
        const int8_t FLUSH_DELAY = 100;
        const int8_t FLUSH_COUNT = 3;

        if (mainKeyStatus.keyPressed && !mainKeyStatus.keyPressedPrev) {
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
    } else if (page == 6) {
        if (pageNeedInit) {
            M5.Lcd.fillScreen(BLACK);
            textOutAligned("Wifi camera receiver", 80, 0, 1, WHITE, TC_DATUM);
            textOut("Press main key to start", 0, 20, 0, WHITE);
            this->refreshInterval = 500;
            this->allowLightSleep=0;
            if (mainKeyStatus.keyPressed) {
                WiFi.mode(WIFI_MODE_STA);
                pageNeedInit = 0;
            };
            return;
        };

        while(WiFi.status() != WL_CONNECTED) {
           WiFi.begin("ESP32 Camera", "");
            M5.Lcd.fillScreen(BLACK);
            textOut("Connecting...", 0, 0, 0, WHITE);
          ESP_LOGI("cam","Wifi connecting..");
          frameCount=0;
          lastTime=millis();
           delay(1000);
          
        };

        if (this->refreshInterval > 50)
            this->refreshInterval = 20;


        updateFrame(client);
        frameCount++;
        //fps计数器
        if(millis()-lastTime>=1000){
          fps=frameCount;
          frameCount=0;
          lastTime=millis();
        };
        M5.Lcd.setCursor(0,0);
        M5.Lcd.printf("FPS:%d",fps);
        //显示电池状态
        uint16_t vbat = M5.Axp.GetVbatData() * 1.1;
        uint8_t current = M5.Axp.GetIdischargeData() / 2;
        if (current > 0) { //放电状态
            textOut(("Vbat:" + String(vbat) + "mV,Bcur:" + String(current) + "mA").c_str(), 0, 65, 1, GREEN);
        } else {
            current = M5.Axp.GetIchargeData() / 2;
            textOut(("Vbat:" + String(vbat) + "mV,Bcur:" + String(current) + "mA").c_str(), 0, 65, 1, RED);
        };
        
    };
    return;
};

void UIClass::setPage(uint8_t page_)
{

    //this->refreshInterval = this->refreshIntervals[page_];
    this->allowLightSleep=1;
    this->pageNeedInit = 1;
    this->page = page_;
};
