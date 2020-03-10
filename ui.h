#ifndef UI_H
#define UI_H
#include <cinttypes>
class UIClass {

public:
    uint16_t refreshInterval;

    uint8_t page;
    bool pageNeedInit;
    bool allowLightSleep;
    void setPage(uint8_t page_);

    void refresh();
    const uint8_t maxPage = 6;

private:
    const uint16_t refreshIntervals[6] = { 500, 500, 500, 100,500,20 }; //刷新间隔(毫秒)
};
#endif
