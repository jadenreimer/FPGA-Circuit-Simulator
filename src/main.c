#include <stdbool.h>
#include "arm.h"

// Colours
#define RED 0xF800
#define BLACK 0x0000
#define BLUE 0x001F
#define GREEN 0x07E0

// Memory
#define PIXEL_BUFF_REG 0xFF203020   //Controls pixel buffer functionality
#define ON_CHIP_MEM 0xC8000000  //First pixel buffer
#define SDRAM_MEM 0xC0000000    //Second pixel buffer

//VGA
#define X_DIM 320
#define Y_DIM 240

volatile int pixel_buffer_start; // global variable

void clear_screen();
void draw_line(int xi, int yi, int xf, int yf, short int line_color);
void plot_pixel(int x, int y, short int line_color);
void swap (int* x, int* y);
void wait_for_vsync();
void clear_line(int xi, int xf, int y);

int main(void){

    init_interrupts();

    // declare other variables
    // short int draw_colour = BLUE;
    volatile int * pixel_ctrl_ptr = (int *)PIXEL_BUFF_REG;

    /* set front pixel buffer to start of FPGA On-chip memory */
    *(pixel_ctrl_ptr + 1) = ON_CHIP_MEM; // first store the address in the back buffer

    /* now, swap the front/back buffers, to set the front buffer location */
    // wait_for_vsync();
    wait_for_vsync();

    /* initialize a pointer to the pixel buffer, used by drawing functions */
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen(); // pixel_buffer_start points to the pixel buffer

    /* set back pixel buffer to start of SDRAM memory */
    *(pixel_ctrl_ptr + 1) = SDRAM_MEM;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer

    while (true){
        //  clear screen
        clear_screen();

        //body

        //  wait for sync
        wait_for_vsync(); // swap front and back buffers on VGA vertical sync
        pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
    }
}

void clear_screen(){
    short int line_colour = BLACK;
    for (int x = 0; x < X_DIM; x++ ){
    	for (int y = 0; y < Y_DIM ; y++ ){
        	plot_pixel(x, y, line_colour);
        }
    }
    return;
}

void draw_line(int x0, int y0, int x1, int y1, short int line_colour){
    int steep = abs(y1 - y0) > abs(x1 - x0);
    int temp = 0;
    if (steep){
		swap(x0, y0);
		swap(x1, y1);
    }
    if (x0 > x1){
    	swap(x0, x1);
		swap(y0, y1);
    }

    int delta_x = x1 - x0;
    int delta_y = abs(y1 - y0);
    int error = -(delta_x / 2);
    int y = y0;
    int y_step = -1;

    if (y0 < y1){
		y_step = 1;
    }

    for (int i = x0; i < x1; i++){
        if (steep){
            plot_pixel(y, i, line_colour);
        }
        else{
            plot_pixel(i, y, line_colour);
        }
        error += deltay;

        if (error >= 0){
            y += y_step;
            error -= delta_x;
        }
    }
}

void plot_pixel(int x, int y, short int line_color){
    *(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = line_color;
}

void swap (int* x, int* y){
    int temp = *x;
    *x = *y;
    *y = temp;
}

void wait_for_vsync(){
    volatile int * pixel_ctrl_ptr = (int *)PIXEL_BUFF_REG;
    register int status;
    *pixel_ctrl_ptr = 1;    //start synchronization
    status = *(pixel_ctrl_ptr + 3); //read s vlaue
    while ((status & 0x01) != 0){
       status = *(pixel_ctrl_ptr + 3); //read s vlaue
    }
}

void clear_line(int xi, int xf, int y){
    //clears horizontal line
    if (xi > xf){
        swap(&xi, &xf);
    }
    for (int x = xi; x <= xf; x++){
        plot_pixel(x, y, BLACK);
 	}
    return;
}

void draw_square(int x, int y, short int color){
    for (int i = x ; i < (x+5); i++){
        for (int j = y; j < (y+5); j++){
            plot_pixel(i, j, color);
        }
    }
}