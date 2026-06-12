#ifndef __ACPI_H__
#define __ACPI_H__
#include "multiboot2.h"
struct acpi_rsdp{
	unsigned char signature[8];
	unsigned char checksum;
	unsigned char oem_id[6];
	unsigned char revision;
	unsigned int rsdt_address;
	unsigned int length;
	unsigned long xsdt_address;
	unsigned char extended_checksum;
	unsigned char reserved[3];
}__attribute__((packed));
struct acpi_table_header{
	unsigned char signature[4];
	unsigned int length;
	unsigned char revision;
	unsigned char checksum;
	char oem_id[6];
	char oem_table_id[8];
	unsigned int oem_revision;
	unsigned int creator_id;
	unsigned int creator_revision;
}__attribute__((packed));
struct acpi_xsdt{
	struct acpi_table_header header;
	unsigned long table_ptrs[];
}__attribute__((packed));
struct acpi_madt{
	struct acpi_table_header header;
	unsigned int local_apic_address;
	unsigned int flags;
	unsigned char entries[];
}__attribute__((packed));
struct acpi_hpet{
	struct acpi_table_header header;
	unsigned char hardware_rev_id;
	unsigned char comparator_count:5,
				  counter_size:1,
				  reserved:1,
				  legacy_replacement:1;
	unsigned short pci_vendor_id;
	unsigned char address_space_id;    // 0 - system memory, 1 - system I/O
    unsigned char register_bit_width;
    unsigned char register_bit_offset;
    unsigned char reserved_1;
    unsigned long address;
	unsigned char hpet_number;
	unsigned short minimum_tick;
	unsigned char page_protection;
}__attribute__((packed));
extern void ACPI_init(struct multiboot2_tag_acpi_new* acpi_tag);
extern unsigned char x2_provided;
extern struct acpi_madt* madt_addr;
extern struct acpi_hpet* hpet;
#endif
