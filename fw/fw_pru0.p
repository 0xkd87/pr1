#define BASE_ADDR_DATARAM 0x0


//Definition and mapping of the DATA RAM structure
.struct __xMap
	.u32 Idx 		//R1
	.u32 SigFromA8 		//R2
	.u32 TimeStamp 		//R3
	.u32 TimeStamp_Past 	//R4
	.u32 dT 		//R5
.ends

//assign structure to Local registers
.assign __xMap, R1,R5, xMap

#define PRU0_ARM_INTERRUPT 19



.origin 0
.entrypoint 0

    // Add the 2 numbers at DATARAM[0], DATARAM[4] and store
    // into DATARAM[8]
//    mov     r0, 0               // r0 = address of numbers to add
                                // (0 is the start of PRU DATARAM)
//    lbbo    r1, r0, 0, 8        // load 8 bytes = 2 words into r1, r2
//    add     r3, r1, r2          // r3 = r1 + r2
 //   sbbo    r3, r0, 8, 4        // store 4 byte result at addr 8
    // Trigger INTC on host
//    mov     r31.b0, PRU0_ARM_INTERRUPT+16
//===============================================================
Initial:	
		mov     r0, BASE_ADDR_DATARAM  // Address offset
		lbbo    r1, r0, 0, 40 // Load 40 bytes to local registers


_Start:		mov     r0, 0  // Address offset
		lbbo    xMap.SigFromA8, r0, 4, 4 // Load 4 bytes with offset 4 from base address [0]
		ADD	xMap.TimeStamp, xMap.TimeStamp, 4 // +4 (stall) cycles at this point
		QBBC 	_Start, xMap.SigFromA8.t0

SigFromHostDetected:
		ADD	xMap.TimeStamp, xMap.TimeStamp, 2 // 2 stall cycles at this instruction
		SUB	xMap.dT, xMap.TimeStamp, xMap.TimeStamp_Past


		CLR 	xMap.SigFromA8.t0  //clear the flag which host has set
		ADD	xMap.Idx, xMap.Idx, 1

		
		sbbo    r1, r0, 0, 40        // store 40 byte result at addr 0
		MOV	xMap.TimeStamp_Past,xMap.TimeStamp

		mov     r31.b0, PRU0_ARM_INTERRUPT+16 //send out event
		ADD	xMap.TimeStamp, xMap.TimeStamp, 7 // 7 stall cycles at this instruction + -1 jump
		JMP 	_Start


DONE: 		mov     r31.b0, PRU0_ARM_INTERRUPT+16
    		halt

