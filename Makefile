CFLAGS := -mcmodel=large -fno-builtin -g -m64
system: head.o main.o printk.o memory.o entry.o trap.o APIC.o interrupt.o task.o cpu.o keyboard.o mouse.o SMP.o APU_boot.o ACPI.o
	ld -b elf64-x86-64 -z muldefs -o system head.o main.o printk.o memory.o entry.o trap.o APIC.o interrupt.o task.o cpu.o keyboard.o mouse.o SMP.o ACPI.o APU_boot.o -T kernel.lds
SMP.o: SMP.c
	gcc $(CFLAGS) -c SMP.c -o SMP.o
ACPI.o: ACPI.c
	gcc $(CFLAGS) -c ACPI.c -o ACPI.o
APU_boot.o: APU_boot.S
	gcc -E APU_boot.S > APU_boot.s
	as -o APU_boot.o APU_boot.s
mouse.o: mouse.c
	gcc $(CFLAGS) -c mouse.c -o mouse.o
keyboard.o: keyboard.c
	gcc $(CFLAGS) -c keyboard.c -o keyboard.o
APIC.o: APIC.c
	gcc $(CFLAGS)  -c APIC.c -o APIC.o
interrupt.o: interrupt.c
	gcc $(CFLAGS) -c interrupt.c
cpu.o: cpu.c
	gcc $(CFLAGS) -c cpu.c
task.o: task.c
	gcc $(CFLAGS)  -c task.c
memory.o: memory.c
	gcc $(CFLAGS)  -c memory.c
printk.o: printk.c
	gcc $(CFLAGS)  -c printk.c
main.o: main.c
	gcc $(CFLAGS)  -c main.c
trap.o: trap.c
	gcc $(CFLAGS)  -c trap.c
entry.o: entry.S
	gcc -E entry.S > entry.s
	as -o entry.o entry.s
head.o: head.S
	gcc -E head.S > head.s
	as -o head.o head.s
clean:
	rm -rf *.o *.s~ *.s *.S~ *.c~ *.h~ system  Makefile~ Kernel.lds~ KERNEL.BIN 
run: system
	./run.sh
