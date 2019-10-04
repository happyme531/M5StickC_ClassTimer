#ifndef UI_H
#define UI_H
#include <inttypes.h>
class UIClass
{

  public:
    uint16_t refreshInterval;



    uint8_t page;
    void setPage(uint8_t page_);
    void refresh();
    const uint8_t maxPage = 2;

  private:
    
    const uint16_t refreshIntervals[3] = {500, 500, 500};     //刷新间隔(毫秒)
};
#endif
