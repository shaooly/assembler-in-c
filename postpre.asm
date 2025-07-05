MAIN:	mov M1[r2][r7],LENGTH
add r2, STR
LOOP:	jmp END
		prn #-5
		prn #+5
		sub r1, r4
		inc K
		mov sssaada
		bne koa
		mov M1[r3][r3],r3
		bne LOOP
		mov sssaada
		bne koa
		mov r1, r3
END:	mov M1[r3][r3],r3
		bne LOOP
STR:	.string "abcdef"
LENGTH:	.data 6,910,9
K:		.data 22
M1:		.mat [2][3] 1,2,3,4
.string "wastingspaceo npurpose"
