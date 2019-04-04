#include <stdbool.h>
#include <math.h>

// Colours
#define RED 0xF800
#define BLACK 0x0000
#define BLUE 0x001F
#define GREEN 0x07E0
#define WHITE 0xFFFF

// Memory
#define PIXEL_BUFF_REG 0xFF203020   //Controls pixel buffer functionality
#define ON_CHIP_MEM 0xC8000000  //First pixel buffer
#define SDRAM_MEM 0xC0000000    //Second pixel buffer

//VGA
#define X_DIM 320
#define Y_DIM 240
#define GRAPH_LEN 80

// extern short CAP_RAW [120][80];

volatile int pixel_buffer_start; // global variable

void clear_screen();
void draw_line(int xi, int yi, int xf, int yf, short int line_color);
// void draw_image(int x_start, int y_start, int x_size, int y_size, extern short image);
void draw_graph(int x, int y);
void plot_pixel(int x, int y, short int line_color);
void charge_and_voltage(int charge[size], int voltage[size]);
void swap (int* x, int* y);
void wait_for_vsync();
void clear_line(int xi, int xf, int y);

int main(void){

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

    //UI Variables
    int graph_x_dist = 210;
    int graph_y_dist = 110;

    //Graph data
    int voltage[70];
    int charge[70];
    int time_counter = 0;

    while (true){
        //  clear screen
        clear_screen();
        int x_pos = 160-80/2;
        int y_pos = 240/2-120/2;

        //Debugging
        // draw_line(x_pos, 50, 50, y_pos, BLACK);

        //Calculate charge and voltage
        if(time_counter == 5){
            charge_and_voltage(int charge[size], int voltage[size]);
            time_counter = 0;
        }
        //Draw graphs to the right of the circuit
        draw_graph(graph_x_dist, graph_y_dist);
        draw_graph(graph_x_dist, graph_y_dist + GRAPH_LEN + 20); //this one is drawn below the other
        // int i, j;
        // for (i=0; i<120; i++)
        //     for (j=0; j<80; j++)
        //     *(short int *)(pixel_buffer_start + (x_pos + j<<0) + (y_pos + i<<9)) = CAP_RAW[i][j];

        //  wait for sync
        wait_for_vsync(); // swap front and back buffers on VGA vertical sync
        pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
    }
}

// void draw_image(int x_start, int y_start, int x_size, int y_size, extern short image){
//     int i, j;
//     for (i=0; i<y_size; i++)
//         for (j=0; j<x_size; j++)
//         *(short int *)(pixel_buffer_start + (x_start + j<<0) + (y_start + i<<9)) = image[i][j];
// }

void clear_screen(){
    short int line_colour = WHITE;
    for (int x = 0; x < X_DIM; x++ ){
    	for (int y = 0; y < Y_DIM ; y++ ){
        	plot_pixel(x, y, line_colour);
        }
    }
    return;
}

void draw_graph(int x, int y){
    int arrow_len = 5;
    //bars of the graph
    draw_line(x, y, x, y - GRAPH_LEN, BLACK);
    draw_line(x, y, x + GRAPH_LEN, y, BLACK);

    //arrows of the graph
    //top arrow
    draw_line(x, y - GRAPH_LEN, x-arrow_len, y-GRAPH_LEN+arrow_len, BLACK);
    draw_line(x, y - GRAPH_LEN, x+arrow_len, y-GRAPH_LEN+arrow_len, BLACK);
    //right arrow
    draw_line(x + GRAPH_LEN, y, x+GRAPH_LEN-arrow_len, y-arrow_len, BLACK);
    draw_line(x + GRAPH_LEN, y, x+GRAPH_LEN-arrow_len, y+arrow_len, BLACK);
}

void charge_and_voltage(int charge[size], int voltage[size]){
    for (int k = numItems; k > i; k--){
        items[k]=items[k-1];
    }
}

void draw_line(int x0, int y0, int x1, int y1, short int line_colour){
    int is_steep = abs(y1 - y0) > abs(x1 - x0);
    int temp = 0;
    if (is_steep){
		temp = x0;
		x0 = y0;
		y0 = temp;
		temp = x1;
		x1 = y1;
		y1 = temp;
    }
    if (x0 > x1){
    	temp = x0;
		x0 = x1;
		x1 = temp;
		temp = y0;
		y0 = y1;
		y1 = temp;
    }

    int deltax = x1 - x0;
    int deltay = abs(y1 - y0);
    int error = -(deltax / 2);
    int y = y0;
    int y_step = -1;
    if (y0 < y1){
		y_step = 1;
    }
    int i;
    for (i = x0; i < x1; i++){
        if (is_steep){
            plot_pixel(y, i, line_colour);
        }
        else{
            plot_pixel(i, y, line_colour);
        }
        error = error + deltay;
        if (error >= 0){
            y = y + y_step;
            error = error - deltax;
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
