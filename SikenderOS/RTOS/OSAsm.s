;/** OSAsm.c
 ;* @brief Assembly functions for OS
 ;*/
        AREA |.text|, CODE, READONLY, ALIGN=2
        THUMB
        REQUIRE8
        PRESERVE8

        EXTERN  RunPt				; currently running thread
		EXTERN	EndPt				; last TCB pointer 
		EXTERN  NextRunPt			; point to next thread to run
        EXPORT  OS_DisableInterrupts
        EXPORT  OS_EnableInterrupts
		EXPORT	OS_ASM_Signal
		EXPORT	OS_ASM_Wait
        EXPORT  StartOS
		EXPORT  PendSV_Handler


;/** OS_DisableInterrupts
;* This function will disable interrupts for RTOS
;*/
OS_DisableInterrupts
        CPSID   I
        BX      LR


;/** OS_EnableInterrupts
;* This function will enable interrupts of RTOS
;*/
OS_EnableInterrupts
        CPSIE   I
        BX      LR


;/** OS_Signal
;* This function(Spinlock) will signal that a mutual exclusion is taking place in a function
;* @counter counter
;*/
OS_ASM_Signal
	LDREX	R1, [R0]		; R1 = Counter
	ADD		R1, #1			; R1++
	STREX	R2, R1,	[R0]	; Counter = R1, R2 is 0 if successfull
	CMP		R2, #0			; if (R2 != 0) OS_Signal();
	BNE 	OS_ASM_Signal		; SUCCESS?
	BX 		LR				; return


;/** OS_Wait
;* This function(Spinlock) will remove signal for mutual exclusion in function
;* @counter counter
;*/
OS_ASM_Wait
	LDREX	R1,	[R0]		; R1 = Counter
	SUBS	R1,	#1			; R1--
	ITT		PL				; If Then, ok if counter>=0
	STREXPL	R2,	R1,	[R0]	; Counter = R1, R2 is 0 if successfull
	CMPPL	R2,	#0			; SUCCESS?
	BNE		OS_ASM_Wait			; busy wait
	BX		LR				; return


;/** PendSV_Handler
;* This function will handle context switches for TCB
;* @author Sikender & Sijin
;* @date 2/04/2019
;*/
PendSV_Handler
    CPSID   I 					; Make Critical, Use STartcritical?
    PUSH    {R4-R11}    		; Save regs R4-R11, ISR takes care of R0-R3, SP, LR, PC, PSR
    LDR     R0, =RunPt			; R0 is ptr to old thread RunPt
    LDR     R1, [R0]			; RunPt->stackPointer = SP
    STR     SP, [R1]			; save SP t
	
	LDR		R1, =NextRunPt		; Load address of NextRunPtr
	LDR		R1,	[R1]			; R1 =NextRunPt
    ;LDR     R1, [R1,#4]		; R1 =RunPt-> nextPtr
    STR     R1, [R0]  			; RunPt set to R1, RunPt = R1
	
    LDR     SP, [R1]			; SP =RunPt-> sp
    POP     {R4-R11}			; restore regs R4-R11 
    CPSIE   I					; End Critical, Use EndCritical()?
    BX      LR 					; The End



;/** StartOS
;* This function will start running of OS threads
;* @author Sikender & Sijin
;* @date 2/04/2019
;*/
regRun RN 2
StartOS
	LDR     R0, =RunPt			; R0 is address of RunPt, R0 = &RunPt
    LDR     R2, [R0]			; R2 =RunPt
    LDR     SP, [R2]			; SP =RunPt->stackPointer;
    POP     {R4-R11}			; Restore register, match stack defined in main
    POP     {R0-R3}				
    POP     {R12}
								; ignore LR and PSR from initialized stack
	ADD		SP, SP, #4			
    POP     {LR}      			
	ADD		SP, SP, #4			
    CPSIE   I 					
    BX      LR   				; The END


;***************End File******************************************
    ALIGN
    END
		