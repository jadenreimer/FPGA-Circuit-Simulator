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
#define GRAPH_LEN 80
#define ARROW_LEN (GRAPH_LEN/20)

// extern short CAP_RAW [120][80];

volatile int pixel_buffer_start; // global variable

void clear_screen();
void draw_line(int xi, int yi, int xf, int yf, short int line_color);
// void draw_image(int x_start, int y_start, int x_size, int y_size, extern short image);
void draw_graph(int x, int y);
void plot_pixel(int x, int y, short int line_color);

void compute(float & Vs[size],
                float & Ic[size],
                float & Vc[size],
                float & v_stored;
                float amp,
                float freq,
                float phase,
                float cap,
                float res,
                int size,
                float t,
                float t_not,
                bool sw1,
                bool sw2);

void swap (int* x, int* y);
void wait_for_vsync();
void clear_line(int xi, int xf, int y);
void set_switches( bool & sw1, bool & sw2, bool & sw1_ready, bool & sw2_ready );
void tab_over( int & select, bool & tab_ready);


int main(void){
    bool sw1 = false;
    bool sw1_ready = true;
    bool sw2 = false;
    bool sw2_ready = true;

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

    int size = 70;

    //Graph data
    float Vs[size];
    float Ic[size];
    float Vc[size];
    float v_stored = 0.0;

    //time data
    int tc = 0;
    float t = 0.0;
    float t_not = 0.0;

    //circuit data
    float capacitance = 1 * pow(10, -6);
    float resistance = 1000;

    //signal data
    float amp = 1;
    float freq = 1;
    float phase = 0;

    int select = 0;
    bool tab_ready = true;
    bool type_ready = true;

    std::vector<float> circuit_data = {amp, freq, phase, capacitance, resistance};
    std::vector<float> temp_circuit_data = {0,0,0,0,0};
    int digit = 1;

    while (true){
        //  clear screen
        clear_screen();
        int x_pos = 160-80/2;
        int y_pos = 240/2-120/2;

        //Debugging
        // draw_line(x_pos, 50, 50, y_pos, BLACK);

        //Calculate Ic and Vs
        if(tc == 5){

            compute(Vs[size],
                    Ic[size],
                    Vc[size],
                    v_stored,
                    circuit_data[0],
                    circuit_data[1],
                    circuit_data[2],
                    circuit_data[3],
                    circuit_data[4],
                    size,
                    t,
                    t_not,
                    sw1,
                    sw2);

            t = t + 1.0;
            tc = 0;
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
        tc++;
        pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer

        int sw1_old = sw1;
        int sw2_old = sw2;
        set_switches(sw1, sw2, sw1_ready, sw2_ready);
        if (sw1_old != sw1 || sw2_old != sw2) t_not = t;

        tab_over(select, tab_ready);
        // change_data(select, type_ready, circuit_data, temp_circuit_data);
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

void draw_graph(int x, int y, int size, float values[size]){
    //bars of the graph
    draw_line(x, y + GRAPH_LEN, x, y - GRAPH_LEN, BLACK);
    draw_line(x, y, x + GRAPH_LEN, y, BLACK);

    //arrows of the graph
    //top arrow
    draw_line(x, y - GRAPH_LEN, x-ARROW_LEN, y-GRAPH_LEN+ARROW_LEN, BLACK);
    draw_line(x, y - GRAPH_LEN, x+ARROW_LEN, y-GRAPH_LEN+ARROW_LEN, BLACK);
    //right arrow
    draw_line(x + GRAPH_LEN, y, x+GRAPH_LEN-ARROW_LEN, y-ARROW_LEN, BLACK);
    draw_line(x + GRAPH_LEN, y, x+GRAPH_LEN-ARROW_LEN, y+ARROW_LEN, BLACK);
    //bottom arrow
    draw_line(x, y + GRAPH_LEN, x-ARROW_LEN, y+GRAPH_LEN-ARROW_LEN, BLACK);
    draw_line(x, y + GRAPH_LEN, x+ARROW_LEN, y+GRAPH_LEN-ARROW_LEN, BLACK);

    float max = 0;
    //find the max in the array
    for(int i = 0; i < size-1; i++){
        if(max < values[i]) max = values[i];
    }

    //graph the values
    for(int i = 0; i < size-2; i++){
        draw_line((int)(x+i*(GRAPH_LEN-ARROW_LEN)/size), (int)(y-(GRAPH_LEN-ARROW_LEN)*values[i]/max),
                (int)(x+(i+1)*(GRAPH_LEN-ARROW_LEN)/size), (int)(y-(GRAPH_LEN-ARROW_LEN)*values[i+1]/max), RED);
    }
}

void charge_and_voltage(int charge[size], int voltage[size]){
    for (int k = numItems; k > i; k--){
        items[k]=items[k-1];
    }
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

void set_switches( bool & sw1, bool & sw2, bool & sw1_ready, bool & sw2_ready )
{
    volatile int * JTAG_UART_ptr = (int *) 0xFF201000;

    int data;
    data = *(JTAG_UART_ptr);
    if (data == 0x1A && sw1_ready){
        sw1 = !sw1;
        sw1_ready = false;
    }

    else if (data == 0xF01A && !sw1_ready){
        sw1_ready = true;
    }

    if (data == 0x22 && sw2_ready){
        sw2 = !sw2;
        sw2_ready = false;
    }

    else if (data == 0xF022 && !sw2_ready){
        sw2_ready = true;
    }
}

void tab_over( int & select, bool & tab_ready){
    volatile int * JTAG_UART_ptr = (int *) 0xFF201000;

    int data;
    data = *(JTAG_UART_ptr);

    if (data == 0x0D && tab_ready){
        select++;
        tab_ready = false;
    }

    else if (data == 0xF00D && !tab_ready){
        tab_ready = true;
    }

}

// void change_data( int select, bool & type_ready, int & digit, std::vector<float> circuit_data, std::vector<float> temp_circuit_data){
//     volatile int * JTAG_UART_ptr = (int *) 0xFF201000;
//
//     int data;
//     data = *(JTAG_UART_ptr);
//
//     if (type_ready){
//         if (data == 0x45
//         type_ready = false;
//     }
//
// }

void compute(float & Vs[size],
            float & Ic[size],
            float & Vc[size],
            float & v_stored,
            float amp,
            float freq,
            float phase,
            float cap,
            float res,
            int size,
            float t,
            float t_not,
            bool sw1,
            bool sw2)
{
    for (int i; i<size-1; i++){
        Ic[i] = Ic[i+1];
        Vs[i] = Vs[i+1];
    }

    float arg = (freq * (t - t_not)) - phase;
    Vs[size] = amp * sin(arg);

    if (!sw1 && !sw2)
    {
        Ic[size] = 0;
        Vc[size] = Vs[size];
    }

    else if (sw1 && !sw2)
    {
        Ic[size] = cap * amp * freq * cos(arg);
        Vc[size] = Vs[size] * (1 - exp( -(t-t_not) / (res * cap) ) );
        v_stored = Vc[size];
    }

    else if (sw1 && sw2)
    {
        Ic[size] = ((amp * sin(arg) * exp( -(t-t_not) / (res * cap) ) ) / res) + (amp * cap * freq * (1 - exp( -(t-t_not) / (res * cap) ) ) * cos(arg));
        Vc[size] = Vs[size] * (1 - exp( -(t-t_not) / (res * cap) ) );
        v_stored = Vc[size];
    }

    else if (!sw1 && sw2)
    {
        Vc[size] = v_stored *  exp( -(t-t_not) / (res * cap) );
        Ic[size] = - Vc / res;
    }

}
