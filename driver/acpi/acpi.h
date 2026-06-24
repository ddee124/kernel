#ifndef __ACPI_H__
#define __ACPI_H__
#include "lib/multiboot2.h"
struct acpi_rsdp{
	uint8_t signature[8];
	uint8_t checksum;
	uint8_t oem_id[6];
	uint8_t revision;
	uint32_t rsdt_address;
	uint32_t length;
	uint64_t xsdt_address;
	uint8_t extended_checksum;
	uint8_t reserved[3];
}__attribute__((packed));
struct acpi_table_header{
	uint8_t signature[4];
	uint32_t length;
	uint8_t revision;
	uint8_t checksum;
	uint8_t oem_id[6];
	uint8_t oem_table_id[8];
	uint32_t oem_revision;
	uint32_t creator_id;
	uint32_t creator_revision;
}__attribute__((packed));
struct acpi_xsdt{
	struct acpi_table_header header;
	uint64_t table_ptrs[];
}__attribute__((packed));
struct acpi_madt{
	struct acpi_table_header header;
	uint32_t local_apic_address;
	uint32_t flags;
	uint8_t entries[];
}__attribute__((packed));
struct acpi_hpet{
	struct acpi_table_header header;
	uint8_t hardware_rev_id;
	uint8_t comparator_count:5,
				  counter_size:1,
				  reserved:1,
				  legacy_replacement:1;
	uint16_t pci_vendor_id;
	uint8_t address_space_id;    // 0 - system memory, 1 - system I/O
    uint8_t register_bit_width;
    uint8_t register_bit_offset;
    uint8_t reserved_1;
    uint64_t address;
	uint8_t hpet_number;
	uint16_t minimum_tick;
	uint8_t page_protection;
}__attribute__((packed));
extern void ACPI_init(struct multiboot2_tag_acpi_new* acpi_tag);
extern struct acpi_madt* madt_table;
extern struct acpi_hpet* hpet_table;
extern struct acpi_rsdp* rsdp_table;
extern struct acpi_xsdt* xsdt_table;
#endif
