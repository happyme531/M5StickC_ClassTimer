#ifndef UI_H
#define UI_H
#include <inttypes.h>
class UIClass {

public:
    uint16_t refreshInterval;

    uint8_t page;
    bool pageNeedInit;
    void setPage(uint8_t page_);

    void refresh();
    const uint8_t maxPage = 4;

private:
    const uint16_t refreshIntervals[5] = { 500, 500, 500, 10,500 }; //刷新间隔(毫秒)
};
#endif
