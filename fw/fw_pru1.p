//Note For LBBO/SBBO instructions (Burst memory transfer to/from [DATA RAM]..!
// Cycle Per Instruction [CPI] = 1 + Word Counts
// Therefore, 4 byte transfer would take 3 clock cycles (= 1 + 2x word_Count) for execute this instruction..!


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

#define PRU1_ARM_INTERRUPT 20

//=================

.origin 0
.entrypoint 0

Initial:	
		mov     r0, BASE_ADDR_DATARAM  // Address offset
		lbbo    r1, r0, 0, 40 // Load 40 bytes to local registers


_Start:		mov     r0, 0  // Address offset
		lbbo    xMap.SigFromA8, r0, 4, 4 // Load 4 bytes with offset 4 from base address [0]; 
		ADD	xMap.TimeStamp, xMap.TimeStamp, 6 // +4 (stall) cycles at this point
		QBBC 	_Start, xMap.SigFromA8.t0

SigFromHostDetected:
		ADD	xMap.TimeStamp, xMap.TimeStamp, 2 // 2 stall cycles at this instruction
		SUB	xMap.dT, xMap.TimeStamp, xMap.TimeStamp_Past


		CLR 	xMap.SigFromA8.t0  //clear the flag which host has set
		ADD	xMap.Idx, xMap.Idx, 1

		
		sbbo    r1, r0, 0, 40        // store 40 byte result at addr 0
		MOV	xMap.TimeStamp_Past,xMap.TimeStamp

		mov     r31.b0, PRU1_ARM_INTERRUPT+16 //send out event
		ADD	xMap.TimeStamp, xMap.TimeStamp, 7 + 80 // 7 stall cycles at this instruction + -1 jump
		JMP 	_Start


DONE: 		mov     r31.b0, PRU1_ARM_INTERRUPT+16
    		halt

