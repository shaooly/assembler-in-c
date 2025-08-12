START:  mov r1 r2
        add r1, r2,
        sub r1,, r2
        cmp r1,
        mov , r2
LABEL1  inc r1
:       mov r1, r2
        mov r1, #5
        add r2, #10
        sub r3, #-7
        inc #5
        clr #0
        lea #5, r1
        lea r1, r2
        jmp #100
        bne r1
        jsr #50
        stop r1
        rts r2, r3
        inc r1, r2
        clr r1, r2, r3
        mov r1
        add r2
        cmp
        sub
        mov
        lea
        add
        stop r1, r2
        rts LABEL
        mvo r1, r2
        addd r1, r2
        jmpp LABEL
        halt
        push r1
        pop r2
        mov r8, r1
        add r1, r9
        sub r-1, r2
        inc r10
        mov rx, r1
        add r, r2
        sub r1a, r2
        mov #, r1
        mov #5a, r1
        add #12.5, r2
        sub ##5, r3
        cmp #, r1
LOOP:   mov r1, r2
LOOP:   add r1, r2
VERYLONGLABELNAMETHATSHOULDEXCEEDMAXIMUM: inc r1
LAB-EL: mov r1, r2
LAB@EL: add r1, r2
123LAB: sub r1, r2
mov:    inc r1
add:    dec r2
.string hello world
.string "hello world
.string
.string "hello\xworld"
.data
.data 12.5, 10
.data 10a, 20
.data , 30
.data 10,, 20
.data 10, 20,
.entry
.extern
.entry LABEL1, LABEL2
.extern LAB1, LAB2
.entry 123LABEL
.extern -LABEL
        mov DATA[], r1
        mov DATA[, r2
        mov DATA[r1 r2], r3
        mov DATA[5 6], r4
        mov DATA[r1, r2
        mov DATAr1], r3
        mov DATA[5], r1
        mov DATA[LABEL], r2
        mov DATA[r1][r2], r3
        mov DATA[r8], r1
        mov DATA[rx], r2
        mov DATA[r1 ], r2
        mov DATA[ r1], r3
LAB1    mov #5 #10
        push #5, r1, r2
LAB-2:  mvo r8, #10
LAB3 LAB4: addd r1,, r2, r3
        ,
ALONE:
LAB::   inc r1
stop:   stop
        mov 5, r1
        add r1, 10
        mov r1, r2
        mvo r2, r3
        add r1, r2
START:  mov r1 r2
        add r1, r2,
        sub r1,, r2
        cmp r1,
        mov , r2
LABEL1  inc r1
:       mov r1, r2
        mov r1, #5
        add r2, #10
        sub r3, #-7
        inc #5
        clr #0
        lea #5, r1
        lea r1, r2
        jmp #100
        bne r1
        jsr #50
        stop r1
        rts r2, r3
        inc r1, r2
        clr r1, r2, r3
        mov r1
        add r2
        cmp
        sub
        mov
        lea
        add
        stop r1, r2
        rts LABEL
        mvo r1, r2
        addd r1, r2
        jmpp LABEL
        halt
        push r1
        pop r2
        mov r8, r1
        add r1, r9
        sub r-1, r2
        inc r10
        mov rx, r1
        add r, r2
        sub r1a, r2
        mov #, r1
        mov #5a, r1
        add #12.5, r2
        sub ##5, r3
        cmp #, r1
LOOP:   mov r1, r2
LOOP:   add r1, r2
VERYLONGLABELNAMETHATSHOULDEXCEEDMAXIMUM: inc r1
LAB-EL: mov r1, r2
LAB@EL: add r1, r2
123LAB: sub r1, r2
mov:    inc r1
add:    dec r2
.string hello world
.string "hello world
.string
.string "hello\xworld"
.data
.data 12.5, 10
.data 10a, 20
.data , 30
.data 10,, 20
.data 10, 20,
.entry
.extern
.entry LABEL1, LABEL2
.extern LAB1, LAB2
.entry 123LABEL
.extern -LABEL
        mov DATA[][], r1
        mov DATA[[], r2
        mov DATA[r1 r2][r3], r3
        mov DATA[5][6], r4
        mov DATA[r1][r3, r2
        mov DATAr1][r2], r3
        mov DATA[5][r2], r1
        mov DATA[LABEL][r3], r2
        mov DATA[r8], r1
        mov DATA[rx], r2
LAB1    mov #5 #10
        push #5, r1, r2
LAB-2:  mvo r8, #10
LAB3 LAB4: addd r1,, r2, r3
        ,
ALONE:
LAB::   inc r1
stop:   stop
        mov 5, r1
        add r1, 10
        mov r1, r2
        mvo r2, r3
        add r1, r2
