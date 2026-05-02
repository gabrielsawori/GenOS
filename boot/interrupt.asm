global load_idt
global keyboard_isr

extern isr1_handler ; Kita akan buat fungsi C ini nanti

; 1. Fungsi untuk memuat IDT ke CPU
load_idt:
    mov edx, [esp + 4]
    lidt [edx]
    ret

; 2. Interrupt Service Routine (ISR) untuk Keyboard
keyboard_isr:
    pusha           ; Simpan semua register (EAX, EBX, dll) agar aman
    cld             ; Clear direction flag (C standard)
    call isr1_handler ; Panggil handler di C
    popa            ; Kembalikan semua register
    iret            ; Interrupt Return (Kembali ke pekerjaan sebelumnya)

global isr128
extern syscall_handler

; ISR 128 (0x80) - Pintu Gerbang Syscall
isr128:
    cli
    push byte 0      ; Dummy error code agar strukturnya sama
    push byte 128    ; Nomor interupsi (int_no)

    pusha            ; Simpan semua register utama (edi, esi, ebp, esp, ebx, edx, ecx, eax)
    push ds          ; Simpan Data Segment saat ini

    mov ax, 0x10     ; Muat Kernel Data Segment (0x10)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp         ; Jadikan posisi tumpukan (stack) sebagai parameter pointer 'registers_t *regs'
    call syscall_handler
    add esp, 4       ; Bersihkan parameter pointer

    pop ds           ; Kembalikan Data Segment aslinya
    popa             ; Kembalikan semua register utama
    add esp, 8       ; Bersihkan dummy error code & int_no
    sti
    iretd            ; Kembali ke program User Space