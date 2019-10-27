#include "ui.h"
#include "hardware.h"
#include "noisedetection.h"
#include "string"
#include "timeutil.h"
using namespace std;

HTU21D htu21d;
bool initSucceed = 0;
void UIClass::refresh()
{

    if (page == 0) {
        /*
        第一页:时间等基本信息
       */
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
        M5.Lcd.fillScreen(BLACK);
        String tempStr;
        tempStr = String(getNoiseVal());
        textOut(tempStr.c_str(), 0, 0, 2, GREEN);

    } else if (page == 4) {

        if (pageNeedInit) {
            Wire.begin(36, 26,100000);
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
    };
};

void UIClass::setPage(uint8_t page_)
{

    this->refreshInterval = this->refreshIntervals[page_];
    this->pageNeedInit = 1;
    this->page = page_;
};
