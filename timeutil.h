#ifndef TIMEUTIL_H
#define TIMEUTIL_H
#include <Arduino.h>                          //没有这行,编译时String会报错
#include <string>                              //没有这行,编译时string会报错
#include "time.h"
#include "lang_zh.h"

enum class_t {Chinese, Math, English, Physics, Chemistery, Biology, PE, SelfStudy, Unknown};
//课程的显示名字
const String className[] = {"Chi", "Mat", "Eng", "Phy", "Chem", "Bio", "PE", "stu", "nul"}; 

enum classEvent_t {CLASS_READY,CLASS_END,CLASS_BEGIN,LATE_BEGIN,XIEZU_UNSAFE};
//下一个事件的类型:准备上课,下课,上课,迟到,斜足危险(雾)
extern char* eventName[];

const class_t classtable[6][11] = //课程表
{{English, English, Physics, Chemistery, Chemistery, Math, Math, SelfStudy, SelfStudy, SelfStudy, Chinese}, \
  {Math, Math, Chemistery, Biology, Biology, Chinese, Chinese, Physics, SelfStudy, SelfStudy, Physics}, \
  {Physics, Physics, Chinese, Chinese, PE, English, English, Math, Biology, SelfStudy, Math}, \
  {Chemistery, Chemistery, Math, English, English, Biology, Biology, Physics, Chinese, SelfStudy, English}, \
  {Chinese, Chinese, Math, Math, PE, Physics, Physics, English, Chemistery, SelfStudy, Chemistery}, \
  {Math, Physics, English, Chemistery, Unknown, Chinese, Biology, Unknown, Unknown, SelfStudy, Biology}  \
};



//控制食堂电视使用的遥控代码
const uint32_t irCodeNEC[]={0xbd807f,0x2fd48b7,0xfd00ff,0x20250af,0xd8813bce,0x8f7eb14,0xfd48b7,0x58853bce,0xFF38C7,0x80bf3bc4};
const uint32_t irCodeRC5=0x80c;
const uint32_t irCodeSamsung=0x80bf3bc4;

static unsigned int irCode[]={8850,4800,400,750,400,750,400,800,400,750,400,800,400,850,350,850,350,800,350,1950,350,1950,350,1950,350,1950,400,1950,300,2000,300,900,300,2000,300,900,250,2050,300,900,300,900,300,2000,350,700,400,800,400,800,400,1900,350,900,300,2000,350,1950,350,850,300,2000,300,2000,300,1950,350,200};


//用于NTP

#ifndef CLASS_TABLE_SATURDAY 
//(周一到周五的时间安排)
//{7°40′, 8°20′, 8°30′, 9°10′, 9°20′, 10°, 10°30′, 11°10′, 11°20′, 12°, 14°30′, 15°10′, 15°20′, 16°, 16°30′, 17°10′, 17°20′, 18°, 18°40′, 20°, 20°10′, 21°10′, 21°20′, 22°15′};
const uint32_t classTime[]={26280,26400,27480,27600,30000,30480,30600,33000,33480,33600,36000,36480,37680,37800,40200,40680,40800,43200,51600,52080,52200,54600,55080,55200,57600,59280,59400,61800,62280,62400,64800,67200,72000,72600,76200,76800,80100};
const classEvent_t classEventType[]={LATE_BEGIN,CLASS_BEGIN,CLASS_READY,CLASS_BEGIN,CLASS_END,CLASS_READY,CLASS_BEGIN,CLASS_END,CLASS_READY,CLASS_BEGIN,CLASS_END,LATE_BEGIN,CLASS_READY,CLASS_BEGIN,CLASS_END,CLASS_READY,CLASS_BEGIN,CLASS_END,LATE_BEGIN,CLASS_READY,CLASS_BEGIN,CLASS_END,CLASS_READY,CLASS_BEGIN,CLASS_END,CLASS_READY,CLASS_BEGIN,CLASS_END,CLASS_READY,CLASS_BEGIN,CLASS_END,CLASS_BEGIN,CLASS_END,CLASS_BEGIN,CLASS_END,CLASS_BEGIN,CLASS_END};
//const uint32_t classTime[]={27600,30000,30600,33000,36000,37800,40200,40800,43200,52200,54600,55200,57600,59400,61800,62400,64800,67200,72000,72600,76200,76800,80100};
//const uint32_t classEventTime[]={27600,30000,30600,33000,36000,37800,40200,40800,43200,52200,54600,55200,57600,59400,61800,62400,64800,67200,72000,72600,76200,76800,80100};
//时间对应的事件类型
//const classEvent_t classEventType=[]={};
#else
//(周六)
//{8°00′,8°45′,8°55′,9°40′,9°50′,10°35′,10°45′,11°30′,14°30',15°15′,15°25′,16°10′,16°20′,17°00′,18°00′};
const uint32_t classTime[]={28800,31500,32100,34800,35400,38100,38700,41400,52200,54900,55500,58200,58800,61200,64800};
#endif


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

uint8_t getNextClassNum(struct tm timeNow);

#endif
