;****************************************************************************
; 
;                        Zero One Print Server
;  
; ===========================================================================

		INCLUDE	ps.a

		IMPORT	|Image$$RO$$Base|		; 
		IMPORT	|Image$$RO$$Limit|		; End of ROM code (=start of ROM data)
		IMPORT	|Image$$RW$$Base|		; Base of RAM to initialise
		IMPORT	|Image$$RW$$Limit|		;
		IMPORT	|Image$$ZI$$Base|		; Base and limit of area
		IMPORT	|Image$$ZI$$Limit|		; to zero initialise

		AREA    Init, CODE, READONLY

		ALIGN			
		ENTRY    			
    	
				EXPORT  __main  		;// Pseudo C entry point
__main                  					;// This entry point is never actually called.
		
		
;/*******************************************************************************/
;/* Setup interrupt / Exception vectors.                                        */
;/* If the ROM is at address 0, this is just a sequence of branches.            */
;/*******************************************************************************/	  
        B       Reset_Handler				;// Reset Vector,ROM H/W Initialize
        B       SystemUndefinedHandler
        B       SystemSwiHandler
        B       SystemPrefetchHandler
        B       SystemAbortHandler
        B		SystemReserv		; Reserved vector
        B       SystemIrqHandler 
        B       SystemFiqHandler


MajorVersion
		DCB		0x09		; Major Version
MinorVersion
		DCB		0x08		; Minor Version
PSModelNumber
		DCB		0x32		; Model
ReleaseVersion
		DCB		0x72		; Release Version
RDVersion
		DCD		0x00000021	; RD Version
BuildVersion
		DCW		0x0002		; Build Version
FirmwareString
		;STR8132-9.08.50.0002.00000033r-2015/09/29 17:31:37
		DCB		0x53,0x54,0x52,0x38,0x31,0x33,0x32,0x2D,0x39,0x2E
		DCB		0x30,0x38,0x2E,0x35,0x30,0x2E,0x30,0x30,0x30,0x32
		DCB		0x2E,0x30,0x30,0x30,0x30,0x30,0x30,0x33,0x33,0x72
		DCB		0x2D,0x32,0x30,0x31,0x35,0x2F,0x30,0x39,0x2F,0x32
		DCB		0x39,0x20,0x31,0x37,0x3A,0x33,0x31,0x3A,0x33,0x37
		DCB		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
		DCB		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00

		ALIGN

;/*******************************************************************************/
;/* Default Exception Vector Function Definition  							    */	
;/* Consist of Function Call from C-Program 							        */	
;/*******************************************************************************/
		IMPORT	ISRUndefHandler
		IMPORT	ISRPrefetchHandler
		IMPORT	do_BreakHandler
		IMPORT	do_AbortHandler
		IMPORT	ISRIrqHandler
		IMPORT	ISRFiqHandler
		IMPORT 	do_IRQ
		IMPORT	do_FIQ

SystemUndefinedHandler		
		STMFD	sp!, {r0-r12,lr}
;		SUB		r0, lr, #0x4
;		B		ISRUndefHandler
LOOP1	B		LOOP1
		LDMFD	sp!, {r0-r12, pc}^

SystemSwiHandler 
		STMFD	sp!, {r0-r12,lr}
		LDR		r0, [lr, #-4]
		BIC		r0, r0, #0xff000000
		CMP 	r0, #0xE0
		BEQ 	do_BreakHandler
		CMP 	r0, #0xFE
		BEQ 	MakeSYS
		CMP 	r0, #0xFF
		BEQ 	MakeSVC
		LDMFD	sp!, {r0-r12, pc}^
		
MakeSYS
		MRS 	r1, spsr
		BIC 	r1, r1, #MODE_MASK
		ORR 	r2, r1, #SYS_MODE
		MSR 	spsr_c, r2
		LDMFD	sp!, {r0-r12, pc}^
		
MakeSVC
		MRS		r1, spsr
		BIC		r1, r1, #MODE_MASK
		ORR		r2, r1, #SUP_MODE
		MSR		spsr_c, r2
		LDMFD	sp!, {r0-r12, pc}^

SystemPrefetchHandler
		STMFD	sp!, {r0-r12, lr}
;		SUB		r0, lr, #0x8	
;		B		ISRPrefetchHandler
LOOP2	B		LOOP2
		LDMFD	sp!, {r0-r12, lr}
		SUBS	pc, lr, #4

SystemAbortHandler
		STMFD	sp!, {r0-r12, lr}
		SUB		r0, lr, #0xc		
		B		do_AbortHandler
		LDMFD	sp!, {r0-r12, lr}
		SUBS	pc, lr, #8

SystemReserv
		SUBS	pc, lr, #4

SystemIrqHandler
		add	lr, lr, #4	;ZOT716u2
	  	STMFD	sp!, {r0-r12, lr}
		BL		do_IRQ
		LDMFD	sp!, {r0-r12, lr}
		SUBS	pc, lr, #4		

SystemFiqHandler
		STMFD	sp!, {r0-r7, lr}
		BL		do_FIQ
		LDMFD	sp!, {r0-r7, lr}
		SUBS	pc, lr, #4

		EXPORT	SysRemap
SysRemap
		STMFD	sp!, {r0-r7, lr}
		ADRL 	r0, 0x00000000			;RO Source	start
		ADRL 	r1, 0x00004000			;RO Source end
		ORR 	r3, r0, #0x20000000		;Destination					
0		CMP 	r0, r1
		LDRCC	r2, [r0], #4
		STRCC	r2, [r3], #4
		BCC 	%0
		ldr	r0, =0x76000000				;Remap
		ldr	r1, =0x1
		str	r1, [r0]
		LDMFD	sp!, {r0-r7, lr}
		MOV		pc, lr
		
		EXPORT	DoeCos
DoeCos
		ldr	r0, =0x7C000000				;LED OFF
		ldr	r1, =0x0001C000
		str	r1, [r0]		
		LDR	r0, =0x8060					;Go eCos
		MOV	pc, r0	

;/*******************************************************************************/
;/* Reset Entry Point					                                        */
;/*******************************************************************************/
          EXPORT	Reset_Handler
Reset_Handler             					;//Reset Entry Point
			 
;/*******************************************************************************/
;/* System Configuration 					                                    */        
;/*******************************************************************************/
SYSTEM_CONFIGURATION

	;mkl070509: shut down the watchdog, in case the HW reset is enabled.
	ldr	r0, =0x7A00000C
	mov	r1, #0x0
	str	r1, [r0]

	; enable SPI high speed read for booting
	ldr	r0, =0x71000040
	ldr	r1, [r0]
	mov	r2, #0x40000000
	orr	r1, r1, r2
	str	r1, [r0]

	; change SPI clock rate
	ldr	r0, =0x71000048
	mov	r1, #0x1
	str	r1, [r0]

	; set SMC bank0 setting
	ldr	r0, =0x73000000
	; write protect off, bank enable, bus width 8
	ldr	r1, =0x00000002
	str	r1, [r0]

	; set SMC bank0 timing
	ldr	r0, =0x73000004
	ldr	r1, =0x330F0F0F
	str	r1, [r0]

	; disable PLL power-down, Disable I2S,USB Device
	ldr	r0, =0x77000010
	ldr	r1, =0x00000050
	str	r1, [r0]

	; Disable PCI,DMA,IDE;Enable USB, MAC, VIC, DRAM, Static Memory
	ldr	r0, =0x77000000
	ldr	r1, =0x01F01033
	str	r1, [r0]
	
	ldr	r0, =0x77000004
	ldr	r1, =0x02021003
	str	r1, [r0]

	; De-assert all of RESETn, i.e., exit reset state
	ldr	r0, =0x77000008
	ldr	r1, =0xFFFFFFFF
	str	r1, [r0]

	; for Str8133 using SDR and MII interface simultaneously
	; Set pd_25 = 1, and sel_sdr = 1
	ldr	r0, =0x77000024
	ldr	r1, =0x00133723
	str	r1, [r0]

	; Configuration of Memory Interface Configure Register 
	; SDRC: 16-bit mode, SDRAM Module: 16Mb * 16, Little-Endian
	ldr	r0, =0x72000000
	ldr	r1, =0x00000026
	str	r1, [r0]

	; Configuration of Parameter Configure Register
	ldr	r0, =0x72000004
	ldr	r1, =0x00000020
	str	r1, [r0]

	; Configuration of Timing Parameter 0 Register
	ldr	r0, =0x72000010
	ldr	r1, =0x32292A62
	str	r1, [r0]

	; Configuration of Timing Parameter 1 Register
	ldr	r0, =0x72000014
	ldr	r1, =0x140F09C8
	str	r1, [r0]

	ldr	r0, =0x72000018
	ldr	r1, =0x00070029
	str	r1, [r0]

	; Configuration of Power-Up Control Register	!!Under debuging without doing this!!
	ldr	r0, =0x72000008
	ldr	r1, =0x00000001
	str	r1, [r0]

3	ldr	r1, [r0]
	cmp	r1, #0x2
	bne	%3

	; Configuration of Preread Timeout Disable Register	
	ldr	r0, =0x7200001C
	ldr	r1, =0x00000080
	str	r1, [r0]

	; Configuration of Preread Enable Register
	ldr	r0, =0x72000020
	ldr	r1, =0x000000FF
	str	r1, [r0]

	; Enable I/D cache
	mrc	p15, 0, r0, c1, c0, 0	
	ldr	r2, =0x00001004	
	orr	r0, r0, r2
	mcr	p15, 0, r0, c1, c0, 0
	
;/*******************************************************************************/
;/* If we are in User mode, get into SVC mode by SWI call. 					    */
;/*******************************************************************************/   
SetSVC
    	MRS     r0, CPSR
    	AND     r1, r0, #MODE_MASK			; "PSR_Mode_Mask"	
    	TEQ     r1, #SUP_MODE       		; Test if in SVC mode
    	BEQ     INITIALIZE_STACK            ; if not, skip.

;/*******************************************************************************/
;/* Setup temporary SWI vector to enter SVC mode.					            */
;/*******************************************************************************/   		
		SWI 	0xff 						; Set SVC mode /* Call SWI Vector  */


;/*******************************************************************************/
;/* Protection Region Unit Disable							            		*/
;/*******************************************************************************/   		
		LDR	 	r1, =MMU_ON
		MRC 	p15, 0, r0, c1, c0, 0
		BIC		r0, r0, r1
		MCR 	p15, 0, r0, c1, c0, 0
		
;=====================================
; Initialise STACK 
;=====================================
INITIALIZE_STACK 
	
		LDR		r0,	=(INTR_MASK:OR:FIQ_MODE)
		MSR		cpsr_c, r0
		LDR		sp, =FIQ_STACK

		LDR		r0,	=(INTR_MASK:OR:IRQ_MODE)
		MSR		cpsr_c, r0
		LDR		sp, =IRQ_STACK

		LDR		r0,	=(INTR_MASK:OR:ABT_MODE)
		MSR		cpsr_c, r0
		LDR		sp, =ABT_STACK

		LDR		r0,	=(INTR_MASK:OR:UDF_MODE)
		MSR		cpsr_c, r0		
		LDR		sp, =UDF_STACK

		LDR		r0,	=(INTR_MASK:OR:SUP_MODE)
		MSR		cpsr_c, r0
		LDR		sp, =SUP_STACK   ; Change CPSR to SVC mode

;/*******************************************************************************/
;/* Load up the linker define values for the static data Copy					*/
;/*******************************************************************************/
SysInitVars   	
		LDR  	r0, =|Image$$RO$$Limit| 		;// Get pointer to ROM data
    	LDR  	r1, =|Image$$RW$$Base|  		;// and RAM copy
    	LDR  	r3, =|Image$$ZI$$Base|  		;// Zero init base => top of initialised data
    	
    	CMP     r0, r1               			;// Check that they are different
    	BEQ     %6
    	
5   	CMP     r1, r3               			;// Copy initialized data
    	LDRCC   r2, [r0], #4
    	STRCC   r2, [r1], #4
    	BCC     %5
    	
6  	 	LDR     r1, =|Image$$ZI$$Limit| 		;// Top of zero initialized segment
    	MOV     r2, #0
    	
7  		CMP     r3, r1               			;// Zero initialize
    	STRCC   r2, [r3], #4
    	BCC     %7
	
;====================================================
; Now change to user mode and set up user mode stack.
;====================================================
		LDR		r0, =SYS_MODE
		MSR		cpsr_cx, r0
		SWI 	0xFE
		SWI 	0xFE
		LDR		sp, =USR_STACK
		
;/*******************************************************************************/
;/* Call SysInit ! Now we're go into the C Program, It would never return !	    */
;/*******************************************************************************/ 
		IMPORT  ARMMain
		B       ARMMain  						; "System Start !"



;/*******************************************************************************/
;/*  SYSTEM STACK MEMORY SIZE                                                   */ 
;/*******************************************************************************/	
		AREA 	SYS_STACK, NOINIT
		ALIGN

                %       USR_STACK_SIZE
USR_STACK
                %       UDF_STACK_SIZE
UDF_STACK
                %       ABT_STACK_SIZE
ABT_STACK
                %       IRQ_STACK_SIZE
IRQ_STACK
                %       FIQ_STACK_SIZE
FIQ_STACK
                %       SUP_STACK_SIZE
SUP_STACK


	END
