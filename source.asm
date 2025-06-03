MAIN:	mov M1[r2][r7],LENGTH
add r2,STR
LOOP:	jmp END
		prn #-5
		mcro my_mcro
		mov M1[r3][r3],r3
		bne LOOP
		mcroend
		mcro bruh
		mov sssaada
		bne koa
		mcroend
		sub r1, r4
		inc K
		bruh
		my_mcro
		bruh
END:	stop
STR:	.string "abcdef"
LENGTH:	.data 6,-9,15
K:		.data 22
M1:		.mat [2][2] 1,2,3,4