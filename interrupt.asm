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