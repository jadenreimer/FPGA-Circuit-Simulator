
disable_interrupts(){

}

void init_interrupts(){
    short int config/*R0*/, controller/*R1*/;
    //setup interrupts
    __asm
    {
    START_CONFIG:
        LDR	SP, =0xFFFFFFFC
        LDR	config, =0b110110011
        MSR	CPSR, config
        LDR	SP, =0x3FFFFFFC
        BL	CONFIG_GIC       //configure the ARM generic
        LDR	controller, =0xFF200050
        LDR	config, =0xF
        STR config, [controller, #0x8]
        LDR	controller, =0b01010011
        MSR	CPSR_c, controller
        B END   //skip over the CONFIG_GIC after returning

    //interrupt controller
    // Configure the KEY pushbuttons port to generate interrupts
    CONFIG_GIC:
    		LDR	controller, =0xFFFED848		//target ICDIPTRn
            LDR	config, =0x00000100		//target CPU0
            STR	config, [controller]

            LDR	controller, =0xFFFED108		//ICDISERn enable
            LDR	config, =0x00000200		//interrupt enable
            STR	config, [controller]

    		MOV	config, #0x1
    		LDR	controller, =0xFFFED000
    		STR	config, [controller]

     //Enable IRQ interrupts in the ARM processor
    		LDR	controller, =0xFFFEC100
    		LDR	config, =0xFF
    		STR	config, [controller, #4]

    		MOV	config, #0x1
    		STR	config, [controller]

    		BX	LR

        END:
        }
}


void keys_interrupt(){
    short int config, controller;

    __asm
    {
        SERVICE_IRQ:    PUSH    {R0-R7, LR}
            LDR     R4, =0xFFFEC100  //GIC CPU interface base address
            LDR	R5, [R4, #0x0C]  //read the ICCIAR in the CPU
                                             // interface

        KEYS_HANDLER:   CMP     R5, #73          //check the interrupt ID

        UNEXPECTED:	BNE UNEXPECTED
        		BL	KEY_ISR

        EXIT_IRQ:       STR     R5, [R4, #0x10]  //write to the End of Interrupt
                                                //  Register (ICCEOIR)
                        POP     {R0-R7, LR}
                        SUBS    PC, LR, #4       //return from exception



        				.equ	INIT,	0b11010010	//IRQ_DISABLE + INT_DISABLE
                        .equ	INIT2,	0b01010010		//IRQ_ENABLE + INT_DISABLE

        //fix what this guy does
        NEXT:		//test
        		LDR	R6, =NUM

        		STR	R6, [R7]
        		B	EXIT_IRQ

        NUM:	.word	0x0000003f, 0x00000006, 0x0000005B, 0x0000004f, 0x00000066, 0x0000006d, 0x0000007d, 0x00000007, 0x0000007f, 0x00000067, 0x00000077, 0x0000007c, 0x00000039, 0x0000005e, 0x00000079, 0x00000071
    }
}

void config_GIC(){
    //......
}
void service_IRQ(){
    //......
}
/* Undefined instructions */
void SERVICE_UND(){
    while(1){}
}
/* Software interrupts */
void SERVICE_SVC(){
    while(1){}
}
/* Aborted data reads */
void SERVICE_ABT_DATA(){
    while(1){}
}
/* Aborted instruction fetch */
void SERVICE_ABT_INST(){
    while(1){}
}
/* Priority interrupt */
void SERVICE_FIQ(){
    while(1){}
}

void key_ISR(){

    //bits indicating button presses
    short int key_status = 0b0000;

    // KEY_ISR:	//calls ISR for buttons
    //         LDR	R0, =0xFF200050	//HEX3-0
    //         LDR R1, [R0, #0xC]
    //         STR R1, [R0, #0xC]
    //         LDR	R0, =KEY_HOLD
    //         LDR	R2, [R0]
    //         EOR	R1, R2, R1
    //         STR	R1, [R0]
    //
    //         LDR	R0, =0xFF200020
    //         MOV	R2, #0
    //
    // CHECK_0:	MOV	R3, #0b0001
    //         ANDS	R3, R3, R1
    //             BEQ	CHECK_1
    //             MOV	R2, #0b00111111
    //
    // CHECK_1:	MOV	R3, #0b0010
    //             ANDS	R3, R3, R1
    //             BEQ	CHECK_2
    //             MOV	R3, #0b00000110
    //             ORR	R2, R2, R3, LSL #8
    //
    // CHECK_2:	MOV	R3, #0b0100
    //             ANDS	R3, R3, R1
    //             BEQ	CHECK_3
    //             MOV	R3, #0b01011011
    //             ORR	R2, R2, R3, LSL #16
    //
    // CHECK_3:	MOV	R3, #0b1000
    //             ANDS	R3, R3, R1
    //             BEQ	END_KEY_ISR
    //             MOV	R3, #0b01001111
    //             ORR	R2, R2, R3, LSL #24
    //
    // END_KEY_ISR:	STR	R2, [R0]
    //                 BX	LR
}
