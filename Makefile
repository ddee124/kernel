PIC := APIC
CFLAGS := -mcmodel=large -fno-builtin -g -m64
system: head.o main.o printk.o memory.o entry.o trap.o PIC.o interrupt.o task.o cpu.o keyboard.o
	ld -b elf64-x86-64 -z muldefs -o system head.o main.o printk.o memory.o entry.o trap.o PIC.o interrupt.o task.o cpu.o keyboard.o -T kernel.lds
keyboard.o: keyboard.c
	gcc $(CFLAGS) -c keyboard.c -o keyboard.o
ifeq ($(PIC),APIC)
PIC.o: APIC.c
	gcc $(CFLAGS)  -c APIC.c -o PIC.o
else
PIC.o: 8259A.c
	gcc $(CFLAGS)  -c 8259A.c -o PIC.o
endif
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
	gcc $(CFLAGS)  -c main.c -D$(PIC)
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
