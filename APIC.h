#ifndef __APIC_H__
#define __APIC_H__
struct IOAPIC_map{
	unsigned int phy_addr;
	unsigned char* virtual_index_addr;
	unsigned int* virtual_data_addr;
	unsigned int* virtual_EOI_addr;
}ioapic_map;
extern void APIC_init();
extern void IRQ0x20_interrupt();
extern void IRQ0x21_interrupt();
extern void IRQ0x22_interrupt();
extern void IRQ0x23_interrupt();
extern void IRQ0x24_interrupt();
extern void IRQ0x25_interrupt();
extern void IRQ0x26_interrupt();
extern void IRQ0x27_interrupt();
extern void IRQ0x28_interrupt();
extern void IRQ0x29_interrupt();
extern void IRQ0x2a_interrupt();
extern void IRQ0x2b_interrupt();
extern void IRQ0x2c_interrupt();
extern void IRQ0x2d_interrupt();
extern void IRQ0x2e_interrupt();
extern void IRQ0x2f_interrupt();
extern void IRQ0x30_interrupt();
extern void IRQ0x31_interrupt();
extern void IRQ0x32_interrupt();
extern void IRQ0x33_interrupt();
extern void IRQ0x34_interrupt();
extern void IRQ0x35_interrupt();
extern void IRQ0x36_interrupt();
extern void IRQ0x37_interrupt();
#endif
