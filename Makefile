CC = gcc
LD = ld
ASM = nasm

# Flag -I memastikan compiler mencari header (.h) di semua folder penting kita
CFLAGS = -m32 -fno-stack-protector -fno-builtin -I./drivers -I./kernel -I./fs -I./lib -c
LDFLAGS = -m elf_i386 -T link.ld

# Otomatis mencari semua file sumber di folder yang ditentukan
C_SOURCES = $(wildcard kernel/*.c drivers/*.c fs/*.c lib/*.c)
ASM_SOURCES = $(wildcard boot/*.asm)

# Mengonversi daftar file .c dan .asm menjadi .o (objek)
OBJ = $(C_SOURCES:.c=.o) $(ASM_SOURCES:.asm=.o)

# Perintah utama: Rakit dan jalankan di QEMU
run: kernel.bin
	qemu-system-x86_64 -kernel kernel.bin -drive file=hdd.img,format=raw,index=0,media=disk -net nic,model=e1000 -net user

kernel.bin: $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $^\

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

%.o: %.asm
	$(ASM) -f elf32 $< -o $@

# Perintah pembersihan sebelum compile ulang
clean:
	rm -f $(OBJ) kernel.bin