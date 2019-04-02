/*

Example of ARM and C code together

This is a pretty valuable resource on how to do interrupts in C:
https://www.embedded.com/design/programming-languages-and-tools/4397803/Interrupts-short-and-simple--Part-1---Good-programming-practices

*/

// void ChangeIRQ(unsigned int NewState)
//
// {
//
//        int my_cpsr;                           /* to be used by inline assembler */
//
//        NewState=(~NewState)<<7;           /* invert and shift to bit 7 */
//
//        __asm                              /* invoke the inline assembler */
//
//       {
//
//            MRS my_cpsr, CPSR                /* get current program status */
//
//            BIC my_cpsr, my_cpsr, #0x80      /* clear IRQ disable bit flag */
//
//            ORR my_cpsr, my_cpsr, NewState   /* OR with new value */
//
//           MSR CPSR_c, my_cpsr              /* store updated program status */
//
//       }
//
// }
