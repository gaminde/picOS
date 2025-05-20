.global kernel_main

.section .rodata
    kernel_msg: .asciz "Hello from Kernel\n"

.text
kernel_main:
    ldr x1, =kernel_msg
    bl uart_puts
    ret