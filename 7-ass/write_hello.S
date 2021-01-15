.section .data
    str:    .asciz "Hello There\n"

.global _start
.section .text
_start: 
    movq    $1, %rax
    movq    $1, %rdi
    movq    $str, %rsi
    movq    $12, %rdx
    syscall             /* sys_write */

    movq %rax, %rdi
    movq $60, %rax
    syscall             /* sys_exit */
