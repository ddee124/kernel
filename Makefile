CC = gcc
AS = as
LD = ld
CFLAGS = -O2 -m64 -fno-strict-aliasing -fwrapv -mgeneral-regs-only -finline-functions -mno-red-zone -mcmodel=large -fno-builtin -fno-tree-vectorize -fno-pic -ffreestanding -nostdlib -fno-stack-protector -mno-sse -mno-mmx -mno-80387 -I.
LDFLAGS = -b elf64-x86-64 -z muldefs -T kernel.lds
SRCS_C  = $(shell find . -path ./build -prune -o -name '*.c' -print)
SRCS_S  = $(shell find . -path ./build -prune -o -name '*.S' -print)
OBJS    = $(patsubst ./%.c,build/%.o,$(SRCS_C)) \
          $(patsubst ./%.S,build/%.o,$(SRCS_S))
KERNEL = system

.PHONY: all clean run
all: $(KERNEL)
$(KERNEL): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^
build/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@
build/%.o: %.S
	@mkdir -p $(dir $@)
	$(CC) -E $< > $*.s
	$(AS) -o $@ $*.s
	@rm -f $*.s
clean:
	rm -rf build $(KERNEL)
run: $(KERNEL)
	./run.sh
