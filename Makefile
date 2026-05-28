system: head.o main.o printk.o memory.o entry.o trap.o interrupt.o task.o cpu.o
	ld -b elf64-x86-64 -z muldefs -o system head.o main.o printk.o memory.o entry.o trap.o interrupt.o task.o cpu.o -T kernel.lds -g head.o main.o printk.o memory.o
cpu.o: cpu.c
	gcc -mcmodel=large -fno-builtin -g -m64 -c cpu.c
task.o: task.c
	gcc -mcmodel=large -fno-builtin -g -m64 -c task.c
interrupt.o: interrupt.c
	gcc -mcmodel=large -fno-builtin -g -m64 -c interrupt.c
memory.o: memory.c
	gcc -mcmodel=large -fno-builtin -g -m64 -c memory.c
printk.o: printk.c
	gcc -mcmodel=large -fno-builtin -g -m64 -c printk.c
main.o: main.c
	gcc -mcmodel=large -fno-builtin -g -m64 -c main.c
trap.o: trap.c
	gcc -mcmodel=large -fno-builtin -g -m64 -c trap.c
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
