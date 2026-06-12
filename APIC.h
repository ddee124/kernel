#ifndef __APIC_H__
#define __APIC_H__
struct IOAPIC_map{
	unsigned int phy_addr;
	unsigned char* virtual_index_addr;
	unsigned int* virtual_data_addr;
	unsigned int* virtual_EOI_addr;
}ioapic_map;
extern void APIC_init();
extern void IOAPIC_enable(unsigned long irq);
extern void IOAPIC_disable(unsigned long irq);
extern unsigned long IOAPIC_install(unsigned long irq,void* arg);
extern void IOAPIC_uninstall(unsigned long irq);
extern void IOAPIC_edge_ack(unsigned long irq);
extern void Local_APIC_edge_level_ack(unsigned long irq);
struct APIC_LVT{
	unsigned int vector :8,
	deliver_mode :3,
	reserved_1 :1,
	deliver_status :1,
	polarity :1,
	irr :1,
	trigger :1,
	mask :1,
	timer_mode :2,
	reserved_2 :13;
}__attribute__((packed));
struct IO_APIC_RET_entry{
	unsigned int vector :8,
	deliver_mode :3,
	dest_mode :1,
	deliver_status :1,
	polarity :1,
	irr :1,
	trigger :1,
	mask :1,
	reserved :15;
	union{
		struct{
			unsigned int reserved_1 :24,
			phy_dest :4,
			reserved_2 :4;
		}physical;
		struct{
			unsigned int reserved_1 :24,
			logical_dest :8;
		}logical;
	}destination;
}__attribute__((packed));
struct INT_CMD_REG{
	unsigned int vector :8,
		deliver_mode :3,
		dest_mode :1,
		deliver_status	:1,
		res_1 :1,
		level :1,
		trigger :1,
		res_2 :2,
		dest_shorthand :2,
		res_3 :12;
	union{
		struct{
			unsigned int res_4 :24,
			dest_field :8;
		}apic_destination;
		unsigned int x2apic_destination;
	}destination;
}__attribute__((packed));

//delivery mode
#define	APIC_ICR_IOAPIC_Fixed 0 //LAPIC IOAPIC ICR
#define	IOAPIC_ICR_Lowest_Priority 1 // IOAPIC ICR
#define	APIC_ICR_IOAPIC_SMI 2 //LAPIC IOAPIC ICR
#define	APIC_ICR_IOAPIC_NMI 4 //LAPIC IOAPIC ICR
#define	APIC_ICR_IOAPIC_INIT 5 //LAPIC IOAPIC ICR
#define	ICR_Start_up 6 // ICR
#define	IOAPIC_ExtINT 7 // IOAPIC
//timer mode
#define APIC_LVT_Timer_One_Shot 0
#define APIC_LVT_Timer_Periodic 1
#define APIC_LVT_Timer_TSC_Deadline 2
//mask
#define APIC_ICR_IOAPIC_Masked 1
#define APIC_ICR_IOAPIC_UN_Masked 0
//trigger mode
#define APIC_ICR_IOAPIC_Edge 0
#define APIC_ICR_IOAPIC_Level 1
//delivery status
#define APIC_ICR_IOAPIC_Idle 0
#define APIC_ICR_IOAPIC_Send_Pending 1
//destination shorthand
#define ICR_No_Shorthand 0
#define ICR_Self 1
#define ICR_ALL_INCLUDE_Self 2
#define ICR_ALL_EXCLUDE_Self 3
//destination mode
#define ICR_IOAPIC_DELV_PHYSICAL 0
#define ICR_IOAPIC_DELV_LOGIC 1
//level
#define ICR_LEVEL_DE_ASSERT 0
#define ICR_LEVEL_ASSERT 1
//remote irr
#define APIC_IOAPIC_IRR_RESET 0
#define APIC_IOAPIC_IRR_ACCEPT 1
//pin polarity
#define APIC_IOAPIC_POLARITY_HIGH 0
#define APIC_IOAPIC_POLARITY_LOW 1
#endif
