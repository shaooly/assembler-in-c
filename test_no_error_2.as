; Data and string definitions
STR:    .string "OK"
DATA:   .data 5, -2, 100

; Code section
START:  mov    r2, r1
        add    r1, DATA
        lea    STR, r2
        jsr    EXTFUNC
        cmp    r1, #5
        bne    NOTEQUAL

        prn    r2
        stop

; Another comment
  NOTEQUAL:      sub    #1, r1
        prn    r1

EXTFUNC: stop
