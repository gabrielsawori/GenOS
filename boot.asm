; boot.asm
bits 32
section .multiboot
    dd 0x1BADB002             ; Magic number untuk bootloader
    dd 0x0                    ; Flags
    dd - (0x1BADB002 + 0x0)   ; Checksum

section .text
global start
extern kernel_main            ; Kita akan memanggil fungsi ini dari C

start:
    cli                       ; Matikan interrupts (sementara)
    mov esp, stack_space      ; Set stack pointer
    call kernel_main          ; Panggil kernel utama
    hlt                       ; Halt CPU jika kernel berhenti

section .bss
resb 8192                     ; Reservasi 8KB untuk stack
stack_space: