#include "timeutil.h"
#include <WiFi.h>
#include <ctime>
#include "hardware.h"

const char* wifiSSID_ = "bu~zhun~ceng~wang(2.4G)";
const char* wifiPassword_ = "zlq13834653953";


RTC_DateTypeDef getLunarDate(struct tm time, int *daysPerMonth) {
  RTC_DateTypeDef lunarDate;
  lunarDate.Year = time.tm_year + 1900 + 2697;  //黄帝纪年:2020年->4717年
 int year = time.tm_year+1900,
    month = time.tm_mon+1,
    day = time.tm_mday;
    int bySpring,bySolar;
    int index,flag;

    
    //bySpring 记录春节离当年元旦的天数。
    //bySolar 记录阳历日离当年元旦的天数。
    if( ((lunar200y[year-1901] & 0x0060) >> 5) == 1)
        bySpring = (lunar200y[year-1901] & 0x001F) - 1;
    else
        bySpring = (lunar200y[year-1901] & 0x001F) - 1 + 31;
    bySolar = monthTotal[month-1] + day - 1;
    if( (!(year % 4)) && (month > 2))
        bySolar++;
    
    //*daysPerMonth记录大小月的天数 29 或30
    //index 记录从哪个月开始来计算。
    //flag 是用来对闰月的特殊处理。
    
    //判断阳历日在春节前还是春节后
    if (bySolar >= bySpring) {//阳历日在春节后（含春节那天）
        bySolar -= bySpring;
        month = 1;
        index = 1;
        flag = 0;
        if( ( lunar200y[year - 1901] & (0x80000 >> (index-1)) ) ==0)
            *daysPerMonth = 29;
        else
            *daysPerMonth = 30;
        while(bySolar >= *daysPerMonth) {
            bySolar -= *daysPerMonth;
            index++;
            if(month == ((lunar200y[year - 1901] & 0xF00000) >> 20) ) {
                flag = ~flag;
                if(flag == 0)
                    month++;
            }
            else
                month++;
            if( ( lunar200y[year - 1901] & (0x80000 >> (index-1)) ) ==0)
                *daysPerMonth=29;
            else
                *daysPerMonth=30;
        }
        day = bySolar + 1;
    }
    else {//阳历日在春节前
        bySpring -= bySolar;
        year--;
        month = 12;
        if ( ((lunar200y[year - 1901] & 0xF00000) >> 20) == 0)
            index = 12;
        else
            index = 13;
        flag = 0;
        if( ( lunar200y[year - 1901] & (0x80000 >> (index-1)) ) ==0)
            *daysPerMonth = 29;
        else
            *daysPerMonth = 30;
        while(bySpring > *daysPerMonth) {
            bySpring -= *daysPerMonth;
            index--;
            if(flag == 0)
                month--;
            if(month == ((lunar200y[year - 1901] & 0xF00000) >> 20))
                flag = ~flag;
            if( ( lunar200y[year - 1901] & (0x80000 >> (index-1)) ) ==0)
                *daysPerMonth = 29;
            else
                *daysPerMonth = 30;
        }
        
        day = *daysPerMonth - bySpring + 1;
    }
    lunarDate.Date = day;
    lunarDate.Month = month;
    /* 
    if(month == ((lunar200y[year - 1901] & 0xF00000) >> 20))
        lunar.reserved = 1;
    else
        lunar.reserved = 0;
        */
    return lunarDate;
};

//将类似{4,1,2,3}的关系转换为{1,2,3,4}
// i:索引
// total:数组总长
uint8_t cyclem1(uint8_t i, uint8_t total) {
  if (i == 0) {
    return total - 1;
  } else {
    return i - 1;
  }
}
uint8_t yearTianGan(uint16_t year) {
  int i;
  i = year - 3;
  return cyclem1(i % 10, 10);
}
uint8_t yearDiZhi(uint16_t year) {
  int i;
  i = year - 3;
  return cyclem1(i % 12, 12);
}
uint8_t monthTianGan(uint8_t year, uint8_t month) {
  int i;
  i = (((year - 3)%10) % 5) * 2 + 1 + month - 2;
  i = i % 10;
  return cyclem1(i, 10);
}
uint8_t monthDiZhi(uint8_t month) {
  if (month + 1 >= 11) {
    return cyclem1(month - 11, 12);
  } else {
    return cyclem1(month + 1, 12);
  };
};

uint8_t dayTianGan(uint16_t y, uint8_t m, uint8_t d) {
  ESP_LOGD("time", "dayTianGaninput:%d,%d,%d", y, m, d);
  int g, c, j;
  if (m <= 2) m += 12;
  c = y / 100;
  y = y % 100;
  g = 4 * c + c / 4 + 5 * y + y / 4 + 3 * (m + 1) / 5 + d - 3;
  j = g % 10;

  return cyclem1(j, 10);
}

uint8_t dayDiZhi(uint16_t y, uint8_t m, uint8_t d) {
  int z, c, i, j;
  if (m <= 2) m += 12;
  if (m % 2)
    i = 6;
  else
    i = 0;
  c = y / 100;
  y = y % 100;
  z = 8 * c + c / 4 + 5 * y + y / 4 + 3 * (m + 1) / 5 + d + 7 + i;
  j = z % 12 + 6;
  if (j > 7) j = j % 12;

  return cyclem1(j, 12);
}

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
  uint8_t retryCount=0;

  while (WiFi.status() != WL_CONNECTED) {
    getKey(KEY_MAIN, &exitKey);
    if (exitKey.keyPressed) goto end;
    delay(50);
  };
  
  do{
    retryCount++;
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0,0);
    M5.Lcd.printf("Retry count:%d",retryCount);
    configTime(8 * 60 * 60, 0, "ntp.ntsc.ac.cn","ntp.aliyun.com");
    
  } while (!getLocalTime(&time)||retryCount>16);
end:
  textOut((string) "NTP end", 0, 10, 1, 0xffffff);
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_INFO
  char buf[40];
  strftime(buf, sizeof(buf), "NTP got time:%g/%m/%d %H:%M:%S", &time);
  ESP_LOGI("NTP","%s",buf);
#endif
 
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

uint8_t getNextEventNum(struct tm timeNow) {
  uint32_t timeNowSec = tm2todaySec(timeNow);
  int i = 0;
  for (i = 0; timeNowSec > classTime[i]; i++);
  return i;
};
