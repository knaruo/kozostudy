.h8300h
.section .text
.global _start
.type   _start,@function

_start:
    # Set _bootstack (defined in the mem map) to the stack pointer
    mov.l   #_bootstack,sp
    # Call _main function
    jsr     @_main

1:
    bra     1b
