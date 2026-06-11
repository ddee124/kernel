#include<stdarg.h>
#include "printk.h"
#include "lib.h"
#define do_div(n,base) ({\
	int __res;\
	__asm__("divq %%rcx":"=a"(n),"=d"(__res):"0"(n),"1"(0),"c"(base));\
	__res;\
})
char buf[4096];
extern inline int is_digit(char ch);
extern inline int strlen(char* s);
void putchar(position* Por,int FRcolor,int BKcolor,unsigned char ch){
	int i=0;int j=0;
	unsigned int* addr=0;
	unsigned char* fontp=0;
	int val=0;
	fontp=font_res[ch];
	for(int i=0;i<16;i++){
		addr=(unsigned int*)((Por->FB_addr)+(Por->pitch)*((Por->YPosition)*(Por->YCharSize)+i)+4*(Por->XPosition)*(Por->XCharSize));
		val=0x100;
		for(int j=0;j<8;j++){
			val=val>>1;
			if(*fontp&val)	*addr=FRcolor;
			else *addr=BKcolor;
			addr++;
		}
		fontp++;
	}
}
int skip_atoi(const char **s){
	int i=0;
	while(is_digit(**s)){
		i=i*10+*((*s)++)-'0';
	}
	return i;
}
static char* number(char* str,long num,int base,int size,int precision,int type){
	char c,sign,tmp[64];
	const char* digits="0123456789ABCDEF";
	int i;
	if(type&SMALL)	digits="0123456789abcdef";
	if(type&LEFT)	type&=(~ZEROPAD);
	if(base<2||base>16)	return 0;
	c=(type&ZEROPAD)?'0':' ';
	sign=0;
	if((type&SIGN)&&num<0)	sign='-',num=-num;
	else    sign=(type&PLUS)?'+':((type&SPACE)?' ':0);
	if(sign)	size--;
	if(type&SPECIAL){
		if(base==16)	size-=2;
		else    if(base==8)	size--;
	}
	i=0;
	if(num==0)	tmp[i++]='0';
	else{
		while(num!=0)	tmp[i++]=digits[do_div(num,base)];
	}
	if(i>precision)	precision=i;
	size-=precision;
	if(!(type&(LEFT+ZEROPAD))){
		while(size-->0)	*(str++)=' ';
	}
	if(sign)	*(str++)=sign;
	if(type&SPECIAL){
		if(base==8)	*(str++)='0';
		if(base==16){
			*(str++)='0';
			*(str++)='x';
		}
	}
	if(!(type&LEFT)){
		while(size-->0)	*(str++)=c;
	}
	while(i<precision--)	*(str++)=0;
	while(i-->0)	*(str++)=tmp[i];
	while(size-->0)	*(str++)=' ';
	return str;
}
int vsprintf(char* buf,const char* fmt,va_list args){
	char *str,*s;
	int flags;
	int field_width;
	int precision;
	int len,i;
	int qualifier;
	for(str=buf;*fmt;fmt++){
		if((*fmt)!='%'){
			*(str++)=*fmt;
			continue;
		}
		flags=0;
		repeat:
			fmt++;
			switch(*fmt){
				case '-':flags|=LEFT;
				goto repeat;
				case '+':flags|=PLUS;
				goto repeat;
				case ' ':flags|=SPACE;
				goto repeat;
				case '#':flags|=SPECIAL;
				goto repeat;
				case '0':flags|=ZEROPAD;
				goto repeat;
			}
		field_width=-1;
		if(is_digit(*fmt))	field_width=skip_atoi(&fmt);
		else{
			if((*fmt)=='*'){
				fmt++;
				field_width=va_arg(args,int);
				if(field_width<0){
					field_width=-field_width;
					flags|=LEFT;
				}
			}
		}
		precision=-1;
		if((*fmt)=='.'){
			fmt++;
			if(is_digit(*fmt)){
				precision=skip_atoi(&fmt);
			}
			else{
				if((*fmt)=='*'){
					fmt++;
					precision=va_arg(args,int);
				}
			}
			if(precision<0)	precision=0;
		}
		qualifier=-1;
		if((*fmt)=='h'||(*fmt)=='l'||(*fmt)=='L'||(*fmt)=='Z'){
			qualifier=(*fmt);
			fmt++;
		}
		switch(*fmt){
			case 'c':
				if(!(flags&LEFT)){
					while(--field_width>0)	*(str++)=' ';
				}
				*(str++)=(unsigned char)va_arg(args,int);
				while(--field_width>0)	*(str++)=' ';
				break;
			case 's':
				s=va_arg(args,char*);
				if(!s)	s='\0';
				len=strlen(s);
				if(precision<0)	precision=len;
				else{
					if(len>precision)	len=precision;
				}
				if(!(flags&LEFT)){
					while(--field_width>0)	*(str++)=' ';
				}
				for(i=0;i<len;i++)	*(str++)=*(s++);
				while(len<field_width--)	*(str++)=' ';
				break;
			case 'o':
				if(qualifier=='l')	str=number(str,va_arg(args,unsigned long),8,field_width,precision,flags);
				else    str=number(str,va_arg(args,unsigned int),8,field_width,precision,flags);
				break;
			case 'p':
				if(field_width==-1){
					field_width=2*sizeof(void*);
					flags|=ZEROPAD;
				}
				str=number(str,(unsigned long)va_arg(args,void*),16,field_width,precision,flags);
				break;
			case 'x':
				flags|=SMALL;
			case 'X':
				if(qualifier=='l')	str=number(str,va_arg(args,unsigned long),16,field_width,precision,flags);
				else    str=number(str,va_arg(args,unsigned int),16,field_width,precision,flags);
				break;
			case 'd':
			case 'i':
				flags|=SIGN;
			case 'u':
				if(qualifier=='l')	str=number(str,va_arg(args,long),10,field_width,precision,flags);
				else    str=number(str,va_arg(args,int),10,field_width,precision,flags);
				break;
			case 'n':
				if(qualifier=='l'){
					long *ip=va_arg(args,long*);
					*ip=(str-buf);
				}
				else{
					int *ip=va_arg(args,int*);
					*ip=(str-buf);
				}
				break;
			case '%':
				*(str++)='%';
				break;
			default:
				*(str++)='%';
				if(*fmt)	*(str++)=*fmt;
				else    fmt--;
				break;
		}
	}
	return str-buf;
}
int color_printk(unsigned int FRcolor,unsigned int BKcolor,const char* fmt,...){
	spin_lock(&Pos.printk_lock);
	int i=0;int count=0;int line=0;va_list args;va_start(args,fmt);
	i=vsprintf(buf,fmt,args);
	va_end(args);
	for(count=0;count<i||line;count++){
		if(line>0){
			count--;
			goto Label_tab;
		}
		if((unsigned char)*(buf+count)=='\n'){
			Pos.YPosition++;
			Pos.XPosition=0;
		}
		else{
			if((unsigned char)*(buf+count)=='\b'){
				Pos.XPosition--;
				if(Pos.XPosition<0){
					Pos.XPosition=Pos.XResolution/Pos.XCharSize-1;
					Pos.YPosition--;
					if(Pos.YPosition<0){
						Pos.YPosition=Pos.YResolution/Pos.YCharSize-1;
					}
				}
				putchar(&Pos,FRcolor,BKcolor,' ');
			}
			else{
				if((unsigned char)*(buf+count)=='\t'){
					line=8-(Pos.XPosition&7);
					Label_tab:
					line--;
					putchar(&Pos,FRcolor,BKcolor,' ');
					Pos.XPosition++;
				}
				else{
					putchar(&Pos,FRcolor,BKcolor,(unsigned char)*(buf+count));
					Pos.XPosition++;
				}
			}
		}
		if(Pos.XPosition>=Pos.XResolution/Pos.XCharSize){
			Pos.YPosition++;
			Pos.XPosition=0;
		}
		if(Pos.YPosition>=Pos.YResolution/Pos.YCharSize){
			Pos.YPosition=0;
		}
	}
	spin_unlock(&Pos.printk_lock);
	return i;
}
