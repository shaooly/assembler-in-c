EXTRALABEL: .entry  STR
.extern EXTLABEL
; comment line
    START:  lea     MAT[r1][r2], r3
        prn     r3
        mov #30, r4
        jmp AFTER
        mov #-1 , r0
        jsr     EXTLABEL
        stop

AFTER:      lea     MAT[r0][r7],  r1
            mov     r1,       r5
            stop

  DATA: .data 1, 2, 3,     4, 5
            STR:    .string "hello"
MAT:    .mat [2][2]  1,2,3,4
