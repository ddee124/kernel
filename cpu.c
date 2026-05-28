#include "cpu.h"
#include "printk.h"
void init_cpu(){
	int i,j;
	unsigned int CpuFacName[4]={0,0,0,0};
	char FactoryName[17]={0};
	get_cpuid(0,0,CpuFacName,CpuFacName+1,CpuFacName+2,CpuFacName+3);
	*(unsigned int*)(FactoryName)=CpuFacName[1];
	*(unsigned int*)(FactoryName+4)=CpuFacName[3];
	*(unsigned int*)(FactoryName+8)=CpuFacName[2];
	FactoryName[12]='\0';
	color_printk(0xff00,0,"%s\t%#010x\t%#010x\t%#010x\n",FactoryName,CpuFacName[1],CpuFacName[3],CpuFacName[2]);
	for(i=0x80000002;i<0x80000005;i++){
		get_cpuid(i,0,CpuFacName,CpuFacName+1,CpuFacName+2,CpuFacName+3);
		*(unsigned int*)(FactoryName)=CpuFacName[0];
		*(unsigned int*)(FactoryName+4)=CpuFacName[1];
		*(unsigned int*)(FactoryName+8)=CpuFacName[2];
		*(unsigned int*)(FactoryName+12)=CpuFacName[3];
		FactoryName[16]='\0';
		color_printk(0xff00,0,"%s",FactoryName);
	}
	color_printk(0xff00,0,"\n");
	get_cpuid(1,0,CpuFacName,CpuFacName+1,CpuFacName+2,CpuFacName+3);
	color_printk(0xff00,0,"Family Code:%#010x,Extended Family:%#010x,Model Number:%#010x,Extended Model:%#010x,Processor Type:%#010x,Stepping ID:%#010x\n",(CpuFacName[0]>>8&0xf),(CpuFacName[0]>>20&0xff),(CpuFacName[0]>>4&0xf),(CpuFacName[0]>>16&0xf),(CpuFacName[0]>>12&0x3),(CpuFacName[0]&0xf));
	get_cpuid(0x80000008,0,CpuFacName,CpuFacName+1,CpuFacName+2,CpuFacName+3);
	color_printk(0xff00,0,"Physical Address size:%08d,Linear Address size:%08d\n",(CpuFacName[0]&0xff),(CpuFacName[0]>>8)&0xff);
	get_cpuid(0,0,CpuFacName,CpuFacName+1,CpuFacName+2,CpuFacName+3);
	color_printk(0xffffff,0,"MAX Basic Operation Code :%#010x\t",CpuFacName[0]);
	get_cpuid(0x80000000,0,CpuFacName,CpuFacName+1,CpuFacName+2,CpuFacName+3);
	color_printk(0xffffff,0,"MAX Extended Operation Code :%#010x\n",CpuFacName[0]);
}
