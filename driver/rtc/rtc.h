#ifndef __RTC_H__
#define __RTC_H__ 1
#include<stdint.h>
struct rtc_time{
	uint32_t year;
	uint8_t month,day,hour,minute,second,weekday;
};
extern void RTC_read(struct rtc_time* t);
extern void RTC_show();
#endif
