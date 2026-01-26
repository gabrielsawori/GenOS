CC = gcc
LD = ld
ASM = nasm
CFLAGS = -m32 -fno-stack-protector -fno-builtin -c
LDFLAGS = -m elf_i386 -T link.ld

# Target Utama
run: kernel.bin
	qemu-system-x86_64 -kernel kernel.bin

# Linker: Menyatukan semua object file termasuk idt.o dan interrupt.o
kernel.bin: boot.o kernel.o screen.o utils.o ports.o hardware.o fs.o idt.o interrupt.o
	$(LD) $(LDFLAGS) -o kernel.bin boot.o kernel.o screen.o utils.o ports.o hardware.o fs.o idt.o interrupt.o

# --- Kompilasi C ---

kernel.o: kernel.c
	$(CC) $(CFLAGS) kernel.c -o kernel.o

screen.o: screen.c
	$(CC) $(CFLAGS) screen.c -o screen.o

utils.o: utils.c
	$(CC) $(CFLAGS) utils.c -o utils.o

ports.o: ports.c
	$(CC) $(CFLAGS) ports.c -o ports.o

hardware.o: hardware.c
	$(CC) $(CFLAGS) hardware.c -o hardware.o

fs.o: fs.c
	$(CC) $(CFLAGS) fs.c -o fs.o

idt.o: idt.c
	$(CC) $(CFLAGS) idt.c -o idt.o

# --- Kompilasi Assembly ---

boot.o: boot.asm
	$(ASM) -f elf32 boot.asm -o boot.o

interrupt.o: interrupt.asm
	$(ASM) -f elf32 interrupt.asm -o interrupt.o

# --- Bersih-bersih ---
clean:
	rm *.o *.bin