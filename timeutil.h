#ifndef TIMEUTIL_H
#define TIMEUTIL_H
#include <Arduino.h>                          //没有这行,编译时String会报错
#include <string>                              //没有这行,编译时string会报错
#include "time.h"
#include "lang_zh.h"
#include <RTC.h>

enum class_t {Chinese = 0, Math, English, Physics, Chemistery, Biology, PE, SelfStudy, Unknown};
//课程的显示名字
const static char* className[] = {str_chinese, str_math,       str_english,
                     str_physics, str_chemistery, str_biology,
                     str_PE,      str_selfStudy,  str_unknown};

enum classEvent_t {CLASS_READY,CLASS_END,CLASS_BEGIN,EVENT_BEGIN,EVENT_END,LATE_BEGIN,XIEZU_UNSAFE};
//下一个事件的类型:准备上课,下课,上课,活动开始,活动结束,迟到,斜足危险(雾)
const static char* eventName[] = {str_classReady, str_classOver, str_classBegin,str_eventBegin,str_eventEnd, str_lateForSchool, str_xieZuRequire};

const class_t classTable[6][12] = //课程表
{{English, English, Physics, Chemistery, Chemistery, Math, Math, SelfStudy, SelfStudy, SelfStudy, Chinese,SelfStudy}, \
  {Math, Math, Chemistery, Biology, Biology, Chinese, Chinese, Physics, SelfStudy, SelfStudy, Physics,SelfStudy}, \
  {Physics, Physics, Chinese, Chinese, PE, English, English, Math, Biology, SelfStudy, Math,SelfStudy}, \
  {Chemistery, Chemistery, Math, English, English, Biology, Biology, Physics, Chinese, SelfStudy, English,SelfStudy}, \
  {Chinese, Chinese, Math, Math, PE, Physics, Physics, English, Chemistery, SelfStudy, Chemistery,SelfStudy}, \
  {Math, Physics, English, Chemistery, Unknown, Chinese, Biology, Unknown, Unknown, SelfStudy, Biology,SelfStudy}  \
};



//控制食堂电视使用的遥控代码
const uint32_t irCodeNEC[]={0xbd807f,0x2fd48b7,0xfd00ff,0x20250af,0xd8813bce,0x8f7eb14,0xfd48b7,0x58853bce,0xFF38C7,0x80bf3bc4};
const uint32_t irCodeRC5=0x80c;
const uint32_t irCodeSamsung=0x80bf3bc4;

static unsigned int irCode[]={8850,4800,400,750,400,750,400,800,400,750,400,800,400,850,350,850,350,800,350,1950,350,1950,350,1950,350,1950,400,1950,300,2000,300,900,300,2000,300,900,250,2050,300,900,300,900,300,2000,350,700,400,800,400,800,400,1900,350,900,300,2000,350,1950,350,850,300,2000,300,2000,300,1950,350,200};

//时间安排
extern uint32_t classTime[]; 
extern classEvent_t classEventType[];
extern uint8_t classCount;
/*
const uint32_t classTime_1_5[]={
  //上午:
  //6:30,6:50,7:10, 7:15, 7:35, 7:38, 7:40, 8:20, 8:28, 8:30, 9:10, 9:18, 9:20, 10:00,10:28,10:30,11:10,11:18,11:20,12:00
  23400,24600,25800,26100,27300,27480,27600,30000,30480,30600,33000,33480,33600,36000,37680,37800,40200,40680,40800,43200,
  //下午:
  //12:20,12:40,12:50,14:20,14:28,14:30,15:10,15:18,15:20,16:00,16:38,16:40,17:20,17:28,17:30,18:10,18:30,18:50,19:00,20:00,20:10,21:10,21:30,23:00,23:30
    44400,45600,46200,51600,52080,52200,54600,55080,55200,57600,59880,60000,62400,62880,63000,65400,66600,67800,68400,72000,72600,76200,77400,82800,84600
};

const classEvent_t classEventType_1_5[]={
  //上午:
  //6:30     ,6:50       ,7:10     ,7:15      ,7:35     ,7:38      , 7:40       ,8:20     ,8:28       ,8:30       ,9:10     ,9:18       ,9:20       ,10:00    ,10:28      ,10:30      ,11:10    ,11:18      ,11:20      ,12:00
  EVENT_BEGIN,EVENT_BEGIN,EVENT_END,LATE_BEGIN,EVENT_END,CLASS_READY,CLASS_BEGIN,CLASS_END,CLASS_READY,CLASS_BEGIN,CLASS_END,CLASS_READY,CLASS_BEGIN,CLASS_END,CLASS_READY,CLASS_BEGIN,CLASS_END,CLASS_READY,CLASS_BEGIN,CLASS_END,
  //下午:
  //12:20  ,12:40    ,12:50      ,14:20     ,14:28      ,14:30      ,15:10    ,15:18      ,15:20      ,16:00     ,16:38     ,16:40      ,17:20    ,17:28      ,17:30      ,18:10    ,18:30    ,18:50    ,19:00      ,20:00    ,20:10      ,21:10    ,21:30      ,23:00    ,23:30
  EVENT_END,EVENT_END,EVENT_BEGIN,LATE_BEGIN,CLASS_READY,CLASS_BEGIN,CLASS_END,CLASS_READY,CLASS_BEGIN,CLASS_END,CLASS_READY,CLASS_BEGIN,CLASS_END,CLASS_READY,CLASS_BEGIN,CLASS_END,EVENT_END,EVENT_END,CLASS_BEGIN,CLASS_END,CLASS_BEGIN,CLASS_END,CLASS_BEGIN,CLASS_END,EVENT_END
};

const uint32_t classTime_6[]={
  //6:30,7:00,7:30 ,8:00 ,8:30 ,9:00 ,11:30,11:40,12:10,12:40,13:00,14:00,14:40,15:00,17:00,18:00,18:30,19:00,21:10
  23400,25200,27000,28800,30600,32400,41400,42000,43800,45600,46800,50400,52800,54000,61200,64800,66600,68400,76200
};
const classEvent_t classEventType_6[]={
  //6:30     ,7:00       ,7:30       ,8:00       ,8:30     ,9:00       ,11:30    ,11:40      ,12:10      ,12:40    ,13:00      ,14:00      ,14:40    ,15:00      ,17:00    ,18:00      ,18:30      ,19:00      ,21:10
  EVENT_BEGIN,EVENT_BEGIN,EVENT_BEGIN,EVENT_BEGIN,EVENT_END,CLASS_BEGIN,CLASS_END,EVENT_BEGIN,EVENT_BEGIN,EVENT_END,EVENT_BEGIN,CLASS_BEGIN,CLASS_END,CLASS_BEGIN,CLASS_END,EVENT_BEGIN,EVENT_BEGIN,CLASS_BEGIN,CLASS_END
};
const uint32_t classTime_7[]={
  //6:30,7:00,7:30 ,8:00 ,8:30 ,9:00 ,11:30,11:40,12:10,12:40,13:00,14:00,14:40,15:00,17:00,17:30,18:10,19:30,23:00
  23400,25200,27000,28800,30600,32400,41400,42000,43800,45600,46800,50400,52800,54000,61200,63000,65400,70200,82800
};
const classEvent_t classEventType_7[]={
  //6:30     ,7:00       ,7:30       ,8:00       ,8:30     ,9:00       ,11:30    ,11:40      ,12:10      ,12:40    ,13:00      ,14:00      ,14:40    ,15:00      ,17:00    ,17:30      ,18:10      ,19:30      ,23:00
  EVENT_BEGIN,EVENT_BEGIN,EVENT_BEGIN,EVENT_BEGIN,EVENT_END,CLASS_BEGIN,CLASS_END,EVENT_BEGIN,EVENT_BEGIN,EVENT_END,EVENT_BEGIN,CLASS_BEGIN,CLASS_END,CLASS_BEGIN,CLASS_END,EVENT_BEGIN,EVENT_BEGIN,EVENT_BEGIN,EVENT_END

};
*/

//#ifndef CLASS_TABLE_SATURDAY 

//(周一到周五的时间安排)
//{7°40′, 8°20′, 8°30′, 9°10′, 9°20′, 10°, 10°30′, 11°10′, 11°20′, 12°, 14°30′, 15°10′, 15°20′, 16°, 16°30′, 17°10′, 17°20′, 18°, 18°40′, 20°, 20°10′, 21°10′, 21°20′, 22°15′};
const uint32_t classTime_1_5[]={26280,26400,27480,27600,30000,30480,30600,33000,33480,33600,36000,36480,37680,37800,40200,40680,40800,43200,51600,52080,52200,54600,55080,55200,57600,59280,59400,61800,62280,62400,64800,67200,72000,72600,76200,76800,80400};
const classEvent_t classEventType_1_5[]={LATE_BEGIN,EVENT_BEGIN,CLASS_READY,CLASS_BEGIN,CLASS_END,CLASS_READY,CLASS_BEGIN,CLASS_END,CLASS_READY,CLASS_BEGIN,CLASS_END,LATE_BEGIN,CLASS_READY,CLASS_BEGIN,CLASS_END,CLASS_READY,CLASS_BEGIN,CLASS_END,LATE_BEGIN,CLASS_READY,CLASS_BEGIN,CLASS_END,CLASS_READY,CLASS_BEGIN,CLASS_END,CLASS_READY,CLASS_BEGIN,CLASS_END,CLASS_READY,CLASS_BEGIN,CLASS_END,CLASS_BEGIN,CLASS_END,CLASS_BEGIN,CLASS_END,CLASS_BEGIN,CLASS_END};
const uint32_t classTime_6[] = {26280,26400,27480,27600,30000,30480,30600,33000,33480,33600,36000,36480,37680,37800,40200,40680,40800,43200,51600,52080,52200,54600,55080,55200,57600,59280,59400,61800,62280,62400,64800,67200,72000,72600,76200,76800,80400};
const classEvent_t classEventType_6[] = {LATE_BEGIN,EVENT_BEGIN,CLASS_READY,CLASS_BEGIN,CLASS_END,CLASS_READY,CLASS_BEGIN,CLASS_END,CLASS_READY,CLASS_BEGIN,CLASS_END,LATE_BEGIN,CLASS_READY,CLASS_BEGIN,CLASS_END,CLASS_READY,CLASS_BEGIN,CLASS_END,LATE_BEGIN,CLASS_READY,CLASS_BEGIN,CLASS_END,CLASS_READY,CLASS_BEGIN,CLASS_END,CLASS_READY,CLASS_BEGIN,CLASS_END,CLASS_READY,CLASS_BEGIN,CLASS_END,CLASS_BEGIN,CLASS_END,CLASS_BEGIN,CLASS_END,CLASS_BEGIN,CLASS_END};
const uint32_t classTime_7[] = {0};
const classEvent_t classEventType_7[] = {(classEvent_t)0};

//const uint32_t classTime[]={27600,30000,30600,33000,36000,37800,40200,40800,43200,52200,54600,55200,57600,59400,61800,62400,64800,67200,72000,72600,76200,76800,80100};
//const uint32_t classEventTime[]={27600,30000,30600,33000,36000,37800,40200,40800,43200,52200,54600,55200,57600,59400,61800,62400,64800,67200,72000,72600,76200,76800,80100};
//时间对应的事件类型
//const classEvent_t classEventType=[]={};
//#else
//(周六)
//{8°00′,8°45′,8°55′,9°40′,9°50′,10°35′,10°45′,11°30′,14°30',15°15′,15°25′,16°10′,16°20′,17°00′,18°00′};
//const uint32_t classTime[]={28800,31500,32100,34800,35400,38100,38700,41400,52200,54900,55500,58200,58800,61200,64800};
//#endif

//农历显示部分
//没什么好办法，查表吧
static const unsigned int lunar200y[199] = {
    0x04AE53,0x0A5748,0x5526BD,0x0D2650,0x0D9544,0x46AAB9,0x056A4D,0x09AD42,0x24AEB6,0x04AE4A,/*1901-1910*/
    0x6A4DBE,0x0A4D52,0x0D2546,0x5D52BA,0x0B544E,0x0D6A43,0x296D37,0x095B4B,0x749BC1,0x049754,/*1911-1920*/
    0x0A4B48,0x5B25BC,0x06A550,0x06D445,0x4ADAB8,0x02B64D,0x095742,0x2497B7,0x04974A,0x664B3E,/*1921-1930*/
    0x0D4A51,0x0EA546,0x56D4BA,0x05AD4E,0x02B644,0x393738,0x092E4B,0x7C96BF,0x0C9553,0x0D4A48,/*1931-1940*/
    0x6DA53B,0x0B554F,0x056A45,0x4AADB9,0x025D4D,0x092D42,0x2C95B6,0x0A954A,0x7B4ABD,0x06CA51,/*1941-1950*/
    0x0B5546,0x555ABB,0x04DA4E,0x0A5B43,0x352BB8,0x052B4C,0x8A953F,0x0E9552,0x06AA48,0x6AD53C,/*1951-1960*/
    0x0AB54F,0x04B645,0x4A5739,0x0A574D,0x052642,0x3E9335,0x0D9549,0x75AABE,0x056A51,0x096D46,/*1961-1970*/
    0x54AEBB,0x04AD4F,0x0A4D43,0x4D26B7,0x0D254B,0x8D52BF,0x0B5452,0x0B6A47,0x696D3C,0x095B50,/*1971-1980*/
    0x049B45,0x4A4BB9,0x0A4B4D,0xAB25C2,0x06A554,0x06D449,0x6ADA3D,0x0AB651,0x093746,0x5497BB,/*1981-1990*/
    0x04974F,0x064B44,0x36A537,0x0EA54A,0x86B2BF,0x05AC53,0x0AB647,0x5936BC,0x092E50,0x0C9645,/*1991-2000*/
    0x4D4AB8,0x0D4A4C,0x0DA541,0x25AAB6,0x056A49,0x7AADBD,0x025D52,0x092D47,0x5C95BA,0x0A954E,/*2001-2010*/
    0x0B4A43,0x4B5537,0x0AD54A,0x955ABF,0x04BA53,0x0A5B48,0x652BBC,0x052B50,0x0A9345,0x474AB9,/*2011-2020*/
    0x06AA4C,0x0AD541,0x24DAB6,0x04B64A,0x69573D,0x0A4E51,0x0D2646,0x5E933A,0x0D534D,0x05AA43,/*2021-2030*/
    0x36B537,0x096D4B,0xB4AEBF,0x04AD53,0x0A4D48,0x6D25BC,0x0D254F,0x0D5244,0x5DAA38,0x0B5A4C,/*2031-2040*/
    0x056D41,0x24ADB6,0x049B4A,0x7A4BBE,0x0A4B51,0x0AA546,0x5B52BA,0x06D24E,0x0ADA42,0x355B37,/*2041-2050*/
    0x09374B,0x8497C1,0x049753,0x064B48,0x66A53C,0x0EA54F,0x06B244,0x4AB638,0x0AAE4C,0x092E42,/*2051-2060*/
    0x3C9735,0x0C9649,0x7D4ABD,0x0D4A51,0x0DA545,0x55AABA,0x056A4E,0x0A6D43,0x452EB7,0x052D4B,/*2061-2070*/
    0x8A95BF,0x0A9553,0x0B4A47,0x6B553B,0x0AD54F,0x055A45,0x4A5D38,0x0A5B4C,0x052B42,0x3A93B6,/*2071-2080*/
    0x069349,0x7729BD,0x06AA51,0x0AD546,0x54DABA,0x04B64E,0x0A5743,0x452738,0x0D264A,0x8E933E,/*2081-2090*/
    0x0D5252,0x0DAA47,0x66B53B,0x056D4F,0x04AE45,0x4A4EB9,0x0A4D4C,0x0D1541,0x2D92B5          /*2091-2099*/
};
static const int monthTotal[13] = {0,31,59,90,120,151,181,212,243,273,304,334,365};

//公历转换为农历
RTC_DateTypeDef getLunarDate(struct tm time,int *daysPerMonth = 0);

//计算年月日对应的天干地支，输入公历时间
uint8_t yearTianGan(uint16_t year);
uint8_t yearDiZhi(uint16_t year);
uint8_t monthTianGan(uint8_t year,uint8_t month);
uint8_t monthDiZhi(uint8_t month);
uint8_t dayTianGan(uint16_t y,uint8_t m,uint8_t d);
uint8_t dayDiZhi(uint16_t y,uint8_t m,uint8_t d);

//十方
enum direction10_t {SOUTH,SOUTHWEST,WEST,NORTHWEST,NORTH,NORTHEAST,EAST,SOUTHEAST,UP,DOWN};

//计算生气的方向
//来源:https://v.youku.com/v_show/id_XMjc3MTU1MTUwNA==.html?spm=a2hbt.13141534.1_2.d_1_27&f=52157989 
//41:30
direction10_t dm_GetLiveDirection(RTC_DateTypeDef lunarDate,uint8_t monthDiZhi);


/*
 * 网络对时
 */
 
struct tm getNTPTime();

/*
 * 把一个struct tm 中的时，分，秒转换为秒（和反向）
 */
uint32_t tm2todaySec(struct tm time);
struct tm todaySec2tm(uint32_t sec);
/*
 * 计算时分秒的差值
 */
struct tm hmsDiff(struct tm time1,struct tm time2);

uint8_t getNextEventNum(struct tm timeNow);

#endif
