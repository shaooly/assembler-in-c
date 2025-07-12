		mov A, r1
		cmp A, r1
		add A, r0
		sub #-10, r1
		lea HELLO, r1
		clr r2
		not r2
		inc r2
		dec Count
		jmp Line
		bne Line
		jsr SUBR
		red r1
		prn r1
		rts
		stop
MAIN:	mov M1[r2][r7],M1[r2][r7]
add r2,STR
add r2, r1
		sub r1, r4
LOOP:	jmp END
		prn #-5
		sub r1, r4
		inc K
		mov sssaada
		stop
		rts
		bne koa
		mov M1[r3][r3],r3
		bne LOOP
		mov sssaada
		stop
		rts
		bne koa
END:		mov M1[r3][r3],r3
		bne LOOP
STR:	.string "abcdef"
LENGTH:	.data 6,-9,15
K:		.data 22
M1:		.mat [2][2] 1,23,4