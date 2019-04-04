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
