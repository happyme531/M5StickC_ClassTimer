#ifndef UI_H
#define UI_H
#include <cinttypes>
//Arduino提供的String和Stream库太弱鸡了，STL YES!
#include <string>
#include <iostream>
#include <sstream>
//暂时还不支持optional
//#include <optional>


class UIClass {

public:
    uint16_t refreshInterval;

    uint8_t page;
    bool pageNeedInit;
    bool allowLightSleep;
    void setPage(uint8_t page_);

    void refresh();
    const uint8_t maxPage = 8;

};
#endif
