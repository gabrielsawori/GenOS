global gdt_flush
global tss_flush

; Memuat GDT baru ke CPU
gdt_flush:
    mov eax, [esp+4]  ; Ambil pointer GDT dari argumen C
    lgdt [eax]        ; Load GDT

    mov ax, 0x10      ; 0x10 adalah offset Kernel Data Segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:.flush   ; 0x08 adalah offset Kernel Code Segment
.flush:
    ret

; Memuat TSS baru ke CPU
tss_flush:
    mov ax, 0x2B      ; 0x2B adalah offset TSS Segment (Entri ke-5 GDT, Ring 3)
    ltr ax            ; Load Task Register
    ret