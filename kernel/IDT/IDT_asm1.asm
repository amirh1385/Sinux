[bits 32]

; -----------------------
; GDT
; -----------------------
GDT_start:
    ; Null descriptor
    dd 0
    dd 0

    ; Code descriptor
code_descriptor:
    dw 0xFFFF          ; Limit low
    dw 0x0000          ; Base low
    db 0x00            ; Base middle
    db 0b10011010      ; Access byte
    db 0b11001111      ; Granularity
    db 0x00            ; Base high

    ; Data descriptor
data_descriptor:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 0b10010010
    db 0b11001111
    db 0x00
GDT_end:

GDT_Descriptor:
    dw GDT_end - GDT_start - 1
    dd GDT_start

; -----------------------
; GDT init
; -----------------------
global init_gdt
init_gdt:
    lgdt [GDT_Descriptor]

    ; Set up segment registers
    mov ax, 0x10        ; Data segment selector (offset در GDT: 2*8 = 0x10)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Flush CS
    jmp 0x08:flush_cs
flush_cs:
    ret

; -----------------------
; Default ISR
; -----------------------
global isr_default_handler
isr_default_handler:
    pushad
    mov al, 'D'
    mov dx, 0x3F8
    out dx, al
    popad
    iretd

; -----------------------
; IRQ0 - Timer
; -----------------------
global irq0_handler
irq0_handler:
    pushad
    mov al, 0x20       ; End of interrupt to PIC
    out 0x20, al
    mov al, '0'
    mov dx, 0x3F8
    out dx, al
    popad
    iretd

; -----------------------
; IRQ1 - Keyboard
; -----------------------
global irq1_keyboard_handler
irq1_keyboard_handler:
    pushad
    in al, 0x60        ; Read scancode
    mov dx, 0x3F8
    out dx, al         ; Print to COM1 for debug
    mov al, 0x20       ; End of interrupt to PIC
    out 0x20, al
    popad
    iretd

; -----------------------
; General Protection Fault
; -----------------------
global gpf
gpf:
    pushad
    mov al, 'G'
    mov dx, 0x3F8
    out dx, al
    popad
    iretd