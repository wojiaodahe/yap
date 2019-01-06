.text
.code 32

.global user_syscall
user_syscall:
	stmfd	r13!, {r0}
	stmfd	r13!, {r1}
	sub		r13, r13, #4
	swi	  	4
	ldmfd	r13!, {r0}
	add		r13, r13, #8
	mov pc, lr
