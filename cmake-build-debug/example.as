mcro M1
    mov r3, r4
    add r2, r1
mcroend

mcro M2
    stop
mcroend

    M1
    M2


mcro M3
    prn #-5
    prn #-7
    prn #-8
mcroend
mov   r1 ,   r2

M3

LABEL1:     .data   5,   10 ,   15
    .string   "Hello,World!"
