#ifndef LANG_ZH_H
#define LANG_ZH_H
// M5StickC的官方库只支持GB2312编码，所以只能另外安排这个GB2312编码的文件存储要用到的中文字符串
const static char *str_now = (char*)"当前", 
     *str_noClassLeft = (char*)"无剩余课程",
     *str_nextEvent = (char*)"下一个事件:", 
     *str_type = (char*)"类型:", 
     *str_classOver = (char*)"课程结束",
     *str_classReady=(char*)"课程预备",
     *str_classBegin=(char*)"课程开始",
     *str_eventBegin=(char*)"活动开始",
     *str_eventEnd=(char*)"活动结束",
     *str_lateForSchool=(char*)"学校迟到",
     *str_xieZuRequire=(char*)"斜足要求",
     *str_battery=(char*)"电池",
     *str_temperature=(char*)"温度",
     *str_chinese=(char*)"语文",
     *str_math=(char*)"数学", 
     *str_english=(char*)"英语", 
     *str_physics=(char*)"物理", 
     *str_chemistery=(char*)"化学", 
     *str_biology=(char*)"生物", 
     *str_PE=(char*)"体育", 
     *str_selfStudy=(char*)"自习", 
     *str_unknown=(char*)"未知",
     *str_nextClass=(char*)"下一节课",
     *str_espTouch=(char*)"ESPTouch配网",
     *str_timeCalibrate=(char*)"联网对时",
     *str_arduinoOTA=(char*)"OTA更新(Arduino)",
     *str_webOTA_Station=(char*)"OTA更新(web,wifi客户端)",
     *str_webOTA_AP=(char*)"OTA更新(web,wifi热点)";

const static char* str_tianGans[10]={"甲","乙","丙","丁","戊","己","庚","辛","壬","癸"};
const static char* str_diZhis[12]={"子","丑","寅","卯","辰","巳","午","未","申","酉","戌","亥"}; 

#endif