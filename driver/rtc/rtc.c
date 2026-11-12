#include "driver/rtc/rtc.h"
#include "kernel/printk.h"
#include "lib/io.h"
/* mc146818 compatible cmos/rtc behind ports 0x70/0x71 */
#define CMOS_ADDR 0x70
#define CMOS_DATA 0x71

static uint8_t cmos_read(uint8_t reg){
	io_out8(CMOS_ADDR,reg);
	return io_in8(CMOS_DATA);
}
static uint32_t cmos_updating(){return cmos_read(0x0a)&0x80;}
static uint8_t bcd_to_bin(uint8_t v){return (v&0x0f)+((v>>4)*10);}
void RTC_read(struct rtc_time* t){
	uint8_t sec,min,hour,day,month,year,century,statb;
	uint64_t i;
	/* wait for the update cycle to finish */
	for(i=0;i<10000000;i++)	if(!cmos_updating())	break;
	/* read until two consecutive second reads match,so the fields come
	   from the same update cycle */
	do{
		sec=cmos_read(0x00);
		min=cmos_read(0x02);
		hour=cmos_read(0x04);
		day=cmos_read(0x07);
		month=cmos_read(0x08);
		year=cmos_read(0x09);
	}while(sec!=cmos_read(0x00));
	statb=cmos_read(0x0b);
	century=cmos_read(0x32);
	if(!(statb&0x04)){		/* bcd */
		sec=bcd_to_bin(sec);
		min=bcd_to_bin(min);
		day=bcd_to_bin(day);
		month=bcd_to_bin(month);
		year=bcd_to_bin(year);
		if(!(statb&0x02)){
			uint8_t pm=hour&0x80;
			hour=bcd_to_bin(hour&0x7f);
			if(pm&&hour<12)	hour+=12;
			if(!pm&&hour==12)	hour=0;
		}
		else	hour=bcd_to_bin(hour);
	}
	else if(!(statb&0x02)){		/* binary,12 hour */
		uint8_t pm=hour&0x80;
		hour&=0x7f;
		if(pm&&hour<12)	hour+=12;
		if(!pm&&hour==12)	hour=0;
	}
	if(century==0||century==0xff)	century=20;
	else if(!(statb&0x04))	century=bcd_to_bin(century);
	t->year=century*100+year;
	t->month=month;
	t->day=day;
	t->hour=hour;
	t->minute=min;
	t->second=sec;
	t->weekday=cmos_read(0x06);
}
void RTC_show(){
	struct rtc_time t;
	RTC_read(&t);
	color_printk(GREEN,BLACK,"[rtc] %04lu-%02lu-%02lu %02lu:%02lu:%02lu\n",
		t.year,t.month,t.day,t.hour,t.minute,t.second);
}
