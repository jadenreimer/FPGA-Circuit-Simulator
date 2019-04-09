#include <stdbool.h>
#include <math.h>

int main (void){
    configure_PS2();
    volatile int* LED_ptr = (int*) 0xFF200000;
    *(LED_ptr) = 0b0101010101;
    int data;
    while (true){
        data = get_jtag();
        if (data==0x7A) *(LED_ptr) = 0b0000011111;
        if (data==0x78) *(LED_ptr) = 0b1111100000;
    }
}

int get_jtag( void ){
    volatile int* PS2_ptr = (int*) 0xFF200100;
    int PS2_data = *(PS2_ptr);
    if (PS2_data & 0x00008000) return (PS2_data & 0xFF);
    else return ('\0');
}

void configure_PS2( void ){
    volatile int* PS2_ptr = (int *) 0xFF200100;
    *(PS2_ptr) = 0xFF;

    *(PS2_ptr + 1) = 0x1;
}
