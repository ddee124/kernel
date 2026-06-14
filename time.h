#ifndef __TIME_H__
#define __TIME_H__
#include "lib.h"
struct time{
	int second;
	int minute;
	int hour;
	int day;
	int month;
	int year;
};
#define CMOS_READ(addr) ({\
	io_out8(0x70,0x80|addr);\
	io_in8(0x71);\
})
int get_cmos_time(struct time* time_){
	//cli();
	do{
		time_->year=CMOS_READ(0x09)+CMOS_READ(0x32)*0x100;
		time_->month=CMOS_READ(0x08);
		time_->day=CMOS_READ(0x07);
		time_->hour=CMOS_READ(0x04);
		time_->minute=CMOS_READ(0x02);
		time_->second=CMOS_READ(0x00);
	}while(time_->second!=CMOS_READ(0x00));
	io_out8(0x70,0x00);
	//sti();
}
#endif
