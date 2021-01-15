.section .data
str:    .string "Hello World\n"

.section .text

.globl main
main:
    pushq   %rbp

    movq    $str, %rdi
    call    printf

    popq    %rbp
    ret
