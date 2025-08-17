SHAOOLY: .data 13, -6, 9
GOODBYE: .data 0, 20
MYSTRING: .string "Hello my name is shaooly"
NEO: .mat [2][2] 1,1,1,1
TRINITY: .mat [2][2] 4,2,1,4

START:  mov #5, r0
		mov NEO[r2][r3], GOODBYE
		cmp SHAOOLY, r1
		cmp r4, TRINITY[r5][r6]
		add SHAOOLY, r3
		mcro BACH
		add SHAOOLY, r4
		add SHAOOLY, r6
		mcroend
		BACH
		BACH
		add r5, TRINITY[r6][r7]
		sub #2, r0
		sub SHAOOLY, GOODBYE
		
; yellow submarine he'aarot

YELLOW: sub NEO[r1][r2], TRINITY[r3][r4]
		sub r6, r7
		lea SHAOOLY, r0
		clr r1
		clr GOODBYE
		clr TRINITY[r2][r3]
		not r2
		inc r3
		inc GOODBYE
PAUL:	lea MYSTRING, r2
		add r1, r2
		dec NEO[r6][r7]
		jmp r5
		rts
		bne GOODBYE
		bne r6
		jsr SHAOOLY
		jsr r7
		red SHAOOLY
		red r0
		prn #9
		prn r5
		stop
