;******************************************************************************       
; 异常向量，本程序中，除Reset和HandleIRQ外，其它异常都没有使用
;******************************************************************************   
	import kernel_main
	import init_system
	import current
	import next_run
	import common_irq_handler
	import sys_call_schedule
	import enable_irq
	import disable_irq
	import OSIntNesting
	export __int_schedule
	export __soft_schedule
	export OS_Start
	export syscall

	import  IRQ_MODE_STACK
	import  FIQ_MODE_STACK
	import  SVC_MODE_STACK
	import  SYS_MODE_STACK


DISABLE_IRQ equ 	0x80
DISABLE_FIQ equ 	0x40
SYS_MOD 	equ		0x1f
IRQ_MOD		equ		0x12
FIQ_MOD		equ		0x11
SVC_MOD		equ	  	0x13
ABT_MOD		equ  	0x17
UND_MOD 	equ  	0x1b
MOD_MASK	equ 	0x1f


	AREA    RESET,CODE,READONLY
	PRESERVE8    ;8 

	IMPORT  |Image$$ER_ROM1$$RO$$Base|   ; Base of ROM code 
	IMPORT  |Image$$ER_ROM1$$RO$$Limit|  ; End of ROM code (=start of ROM data) 
	IMPORT  |Image$$RW_RAM1$$RW$$Base|   ; Base of RAM to initialise 
	IMPORT  |Image$$RW_RAM1$$ZI$$Base|   ; Base and limit of area 
	IMPORT  |Image$$RW_RAM1$$ZI$$Limit|  ; to zero initialise 

_start
  	b   Reset


HandleUndef
    b   HandleUndef 		;0x04: 未定义指令中止模式的向量地址
Handle_swi
    b   HandleSWI			;0x08: 管理模式的向量地址，通过SWI指令进入此模式
HandlePrefetchAbort
    b   HandlePrefetchAbort	; 0x0c: 指令预取终止导致的异常的向量地址
HandleDataAbort
    b  	HandleDataAbort		; 0x10: 数据访问终止导致的异常的向量地址
HandleNotUsed
    b   HandleNotUsed		; 0x14: 保留
Handle_irq
    b   HandleIRQ			; 0x18: 中断模式的向量地址
HandleFIQ
    b   HandleFIQ			; 0x1c: 快中断模式的向量地址

Reset

    msr cpsr_cxsf, #0xd2    ; 进入中断模式
	ldr		r0, =IRQ_MODE_STACK
	ldr		r0, [r0]
	mov		sp, r0

	msr     CPSR_cxsf, #(DISABLE_FIQ :OR: DISABLE_IRQ :OR: SVC_MOD)
	ldr		r0, =SVC_MODE_STACK
	ldr		r0, [r0]
	mov		sp, r0

	msr     CPSR_cxsf, #(DISABLE_FIQ :OR: DISABLE_IRQ :OR: FIQ_MOD)
	ldr		r0, =FIQ_MODE_STACK
	ldr		r0, [r0]
	mov		sp, r0
    
	msr cpsr_cxsf, #0xdf	;系统模式
	ldr		r0, =SYS_MODE_STACK
	ldr		r0, [r0]
	mov		sp, r0

	bl copy_proc_beg

	bl init_system
    
    ldr lr, =halt_loop      ; 设置返回地址
    ldr pc, =kernel_main           ; 调用main函数

copy_proc_beg
	adr	r0, _start
	ldr	r2, BaseOfROM
	cmp	r0, r2
	ldreq	r0, TopOfROM
	beq	InitRam	
	ldr r3, TopOfROM
0	
	ldmia	r0!, {r4-r7}
	stmia	r2!, {r4-r7}
	cmp	r2, r3
	bcc	%B0
	
	sub	r2, r2, r3
	sub	r0, r0, r2				
		
InitRam	
	ldr	r2, BaseOfBSS
	ldr	r3, BaseOfZero	
0
	cmp	r2, r3
	ldrcc	r1, [r0], #4
	strcc	r1, [r2], #4
	bcc	%B0	

	mov	r0,	#0
	ldr	r3,	EndOfBSS
1	
	cmp	r2,	r3
	strcc	r0, [r2], #4
	bcc	%B1
	
	mov pc, lr

halt_loop
    b   halt_loop

HandleIRQ
	STMFD   SP!, {R1-R3}			; We will use R1-R3 as temporary registers
;----------------------------------------------------------------------------
;   R1--SP
;	R2--PC 
;   R3--SPSR
;------------------------------------------------------------------------
	MOV     R1, SP
	ADD     SP, SP, #12             ;Adjust IRQ stack pointer
	SUB     R2, LR, #4              ;Adjust PC for return address to task

	MRS     R3, SPSR				; Copy SPSR (Task CPSR)
	

	;CHANGE_TO_SYS       ;切换到 SYS 模式当前进程是在此模式下运行
    msr     CPSR_cxsf, #(DISABLE_FIQ :OR: DISABLE_IRQ :OR: SYS_MOD)

	STMFD   SP!, {R2}				; Push task''s PC 
	STMFD	sp!, {R3}				; Push Task''s cpsr
	STMFD   SP!, {R4-R12, LR}		; Push task''s LR,R12-R4
	
	LDMFD   R1!, {R4-R6}			; Load Task''s R1-R3 from IRQ stack 
	STMFD   SP!, {R4-R6}			; Push Task''s R1-R3 to SYS stack
	STMFD   SP!, {R0}			    ; Push Task''s R0 to SYS stack
 	
	ldr r0, =current;
	ldr r0, [r0]
	str sp, [r0]
	
    bl common_irq_handler

	bl disable_irq	
	
	ldmfd	sp!, {r0 - r12}
	ldmfd	sp!, {r14}
	stmfd	sp!, {r0}
	mov 	r0,	 sp
	add		sp,  sp, #12
	
	msr     CPSR_cxsf, #(DISABLE_FIQ :OR: DISABLE_IRQ :OR: FIQ_MOD)

	mov 	sp, r0
	ldmfd	sp!, {r0}
	ldmfd	sp!, {r14}
	
   	MSR		SPSR_cxsf, R14
	LDMFD 	SP!, {PC}^


__int_schedule               ;在此模式下把当前进程的r0 - r12 及cpsr入栈保存
	LDR		R0, =current
	LDR		R0, [R0]
	LDR		SP, [R0]

	ldmfd	sp!, {r0 - r12}
	ldmfd	sp!, {r14}
	stmfd	sp!, {r0}
	mov 	r0,	 sp
	add		sp,  sp, #12
	
	msr     CPSR_cxsf, #(DISABLE_FIQ :OR: DISABLE_IRQ :OR: FIQ_MOD)

	mov 	sp, r0
	ldmfd	sp!, {r0}
	ldmfd	sp!, {r14}
	
   	MSR		SPSR_cxsf, R14
	LDMFD 	SP!, {PC}^

__soft_schedule
	STMFD	SP!, {LR}           ;PC
	MRS		R14,  CPSR       	;Push CPSR
	STMFD	SP!, {R14}	
	STMFD	SP!, {R0-R12, LR}   ;R0-R12 LR
		
	LDR		R0, =current
	LDR		R0, [R0]
	STR		SP, [R0]
	
	LDR		R0, =next_run
	LDR		R1, =current
	LDR		R0, [R0]
	STR		R0, [R1]

	LDR		R0, =next_run
	LDR		R0, [R0]
	LDR		SP, [R0]
	
  	ldmfd	sp!, {r0 - r12}
	ldmfd	sp!, {r14}
	stmfd	sp!, {r0}
	mov 	r0,	 sp
	add		sp,  sp, #12
	
	msr     CPSR_cxsf, #(DISABLE_FIQ :OR: DISABLE_IRQ :OR: FIQ_MOD)

	mov 	sp, r0
	ldmfd	sp!, {r0}
	ldmfd	sp!, {r14}
	
   	MSR		SPSR_cxsf, R14
	LDMFD 	SP!, {PC}^

OS_Start
	LDR		R0, =current
	LDR		R0, [R0]
	LDR		SP, [R0]

	ldmfd	sp!, {r0 - r12}
	ldmfd	sp!, {r14}
	stmfd	sp!, {r0}
	mov 	r0,	 sp
	add		sp,  sp, #12
	
	msr     CPSR_cxsf, #(DISABLE_FIQ :OR: DISABLE_IRQ :OR: FIQ_MOD)

	mov 	sp, r0
	ldmfd	sp!, {r0}
	ldmfd	sp!, {r14}
	
   	MSR		SPSR_cxsf, R14
	LDMFD 	SP!, {PC}^

HandleSWI
	STMFD   SP!, {R1-R3}			; We will use R1-R3 as temporary registers
;----------------------------------------------------------------------------
;   R1--SP
;	R2--PC 
;   R3--SPSR
;------------------------------------------------------------------------
	MOV     R1, SP
	ADD     SP, SP, #12             ;Adjust SWI stack pointer
	MOV     R2, LR		            ;Adjust PC for return address to task

	MRS     R3, SPSR				; Copy SPSR (Task CPSR)
	

	;CHANGE_TO_SYS       ;切换到 SYS 模式当前进程是在此模式下运行
    msr     CPSR_cxsf, #(DISABLE_FIQ :OR: DISABLE_IRQ :OR: SYS_MOD)

	STMFD   SP!, {R2}				; Push task''s PC 
	STMFD	sp!, {R3}				; Push Task''s cpsr
	STMFD   SP!, {R4-R12, LR}		; Push task''s LR,R12-R4
	
	LDMFD   R1!, {R4-R6}			; Load Task''s R1-R3 from IRQ stack 
	STMFD   SP!, {R4-R6}			; Push Task''s R1-R3 to SYS stack
	STMFD   SP!, {R0}			    ; Push Task''s R0 to SYS stack
	
	
    bl enable_irq
	ldr		r1, [r13, #72]
	ldr 	r2, [r13, #68]
	ldr	 	r3, [r13, #60]
	ldr		r0, [r3, #-4]
	bic		r0, r0, #0xff000000
    bl sys_call_schedule

	str		r0, [r13, #64]
	bl disable_irq
    
	ldmfd	sp!, {r0 - r12}
	ldmfd	sp!, {r14}
	stmfd	sp!, {r0}
	mov 	r0,	 sp
	add		sp,  sp, #12
	
	msr     CPSR_cxsf, #(DISABLE_FIQ :OR: DISABLE_IRQ :OR: FIQ_MOD)

	mov 	sp, r0
	ldmfd	sp!, {r0}
	ldmfd	sp!, {r14}
	
   	MSR		SPSR_cxsf, R14
	LDMFD 	SP!, {PC}^

syscall
	stmfd	r13!, {r0}
	stmfd	r13!, {r1}
	sub		r13, r13, #4
	swi	  	4
	ldmfd	r13!, {r0}
	add		r13, r13, #8
	mov pc, lr

BaseOfROM  DCD  |Image$$ER_ROM1$$RO$$Base| 
TopOfROM   DCD  |Image$$ER_ROM1$$RO$$Limit| 
BaseOfBSS  DCD  |Image$$RW_RAM1$$RW$$Base| 
BaseOfZero DCD  |Image$$RW_RAM1$$ZI$$Base| 
EndOfBSS   DCD  |Image$$RW_RAM1$$ZI$$Limit| 

current_task_sp
	mov r0, sp
	mov pc, lr

data_abort
	
	ldr sp, =0x31000000
	mrs		r14, spsr
	MSR		cPSR_cxsf, R14
	b .

	end
