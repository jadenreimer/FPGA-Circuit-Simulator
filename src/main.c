#include <stdbool.h>
#include <math.h>
//#include "arm.h"

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
#define ARROW_LEN (GRAPH_LEN/20)

// extern short CAP_RAW [120][80];

volatile int pixel_buffer_start; // global variable

void clear_screen();
void draw_line(int xi, int yi, int xf, int yf, short int line_color);
// void draw_image(int x_start, int y_start, int x_size, int y_size, extern short image);
void draw_graph(int x, int y, int size, float values[size]);
void draw_graph(int x, int y);
void plot_pixel(int x, int y, short int line_color);

void compute(	int size,
             	float *Vs,
             	float *Ic,
            	float *Vc,
             	float *v_stored,
             	float amp,
                float freq,
                float phase,
                float cap,
                float res,
                float t,
                float t_not,
                bool sw1,
             	bool sw2);

void swap (int* x, int* y);
void wait_for_vsync();
void clear_line(int xi, int xf, int y);
void set_switches( bool *sw1, bool *sw2, bool *sw1_ready, bool *sw2_ready );
void tab_over( int *select, int *digit, bool *tab_ready, float *circuit_data, float *temp_circuit_data);
void change_data( int *select, int *digit, bool *type_ready, float *circuit_data, float *temp_circuit_data);;

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
    // float test[30] = {1, 2, 4, 8, 3, 1, 2, 4, 8, 3, 1, 2, 4, 8, 3, 1, 2, 4, 8, 3, 1, 2, 4, 8, 3, 1, 2, 4, 8, 3};

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

    float circuit_data[5] = {amp, freq, phase, capacitance, resistance};
    float temp_circuit_data[5] = {0,0,0,0,0};
    int digit = 0;

    while (true){
        //  clear screen
        //Calculate Ic and Vs
        if(tc == 5){

            compute(size,
                    Vs,
                    Ic,
                    Vc,
                    &v_stored,
                    circuit_data[0],
                    circuit_data[1],
                    circuit_data[2],
                    circuit_data[3],
                    circuit_data[4],
                    t,
                    t_not,
                    sw1,
                    sw2);

            t = t + 1.0;
            tc = 0;

            //Debugging only
            // for(int i=0; i<28; i++){
            //     if(i < 15)
            //     test[i] = sin(i)*test[i+1];
            //     else test[i] = sin(i)/test[i+1];
            // }
            // test[29] = test[28]*2;
            // tc = 0;
        }

        //Draw graphs to the right of the circuit
        draw_graph(200, 120, 70, Vs);
        // draw_graph(graph_x_dist, graph_y_dist + GRAPH_LEN + 20); //this one is drawn below the other
        draw_graph(graph_x_dist, graph_y_dist);
        draw_graph(graph_x_dist, graph_y_dist + GRAPH_LEN + 20); //this one is drawn below the other
        // int i, j;
        // for (i=0; i<120; i++)
        //     for (j=0; j<80; j++)

        int sw1_old = sw1;
        int sw2_old = sw2;
        set_switches(&sw1, &sw2, &sw1_ready, &sw2_ready);
        if (sw1_old != sw1 || sw2_old != sw2) t_not = t;

<<<<<<< HEAD
        // tab_over(select, tab_ready);
        // change_data(select, type_ready, circuit_data, temp_circuit_data);
    }
}

void clear_screen(){
    short int line_colour = WHITE;
    for (int x = 0; x < X_DIM; x++ ){
    	for (int y = 0; y < Y_DIM ; y++ ){
        	plot_pixel(x, y, line_colour);
        }
    }
    return;
}

void draw_graph(int x, int y, int size, float values[size]){
    //bars of the graph
    draw_line(x, y + GRAPH_LEN, x, y - GRAPH_LEN, BLACK);//up-down
    draw_line(x, y, x + GRAPH_LEN, y, BLACK);//horizontal

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
        draw_line((int)(x+i*(GRAPH_LEN-ARROW_LEN)/size), (int)(y-(GRAPH_LEN-ARROW_LEN)*(values[i]/max)),
                (int)(x+(i+1)*(GRAPH_LEN-ARROW_LEN)/size), (int)(y-(GRAPH_LEN-ARROW_LEN)*(values[i+1]/max)), RED);
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
=======
        tab_over(&select, &digit, &tab_ready, circuit_data, temp_circuit_data);
        change_data(&select, &digit, &type_ready, circuit_data, temp_circuit_data);
>>>>>>> 750bca5b6f335aa399854ab21996a766b40421eb
    }
}

void set_switches( bool *sw1, bool *sw2, bool *sw1_ready, bool *sw2_ready )
{
    volatile int * JTAG_UART_ptr = (int *) 0xFF201000;

    int data;
    data = *(JTAG_UART_ptr);

    if (data == 0x1A && (*sw1_ready)){
        *sw1 = !(*sw1);
        *sw1_ready = false;
    }

    else if (data == 0xF01A && !(*sw1_ready)){
        *sw1_ready = true;
    }

    if (data == 0x22 && (*sw2_ready)){
        *sw2 = !(*sw2);
        *sw2_ready = false;
    }

    else if (data == 0xF022 && !(*sw2_ready)){
        *sw2_ready = true;
    }
}

void tab_over( int *select, int *digit, bool *tab_ready, float *circuit_data, float *temp_circuit_data){
    volatile int * JTAG_UART_ptr = (int *) 0xFF201000;

    int data;
    data = *(JTAG_UART_ptr);

    if (data == 0x0D && (*tab_ready)){
        (*select)++;
        if ((*select)>4) (*select) = 0;
        *tab_ready = false;
        *temp_circuit_data = *circuit_data;
    }

    else if (data == 0xF00D && !(*tab_ready)){
        *tab_ready = true;
    }

}

void change_data( int *select, int *digit, bool *type_ready, float *circuit_data, float *temp_circuit_data){
    volatile int * JTAG_UART_ptr = (int *) 0xFF201000;

    int data;
    data = *(JTAG_UART_ptr);

    if (type_ready){
        if (data == 0x45 && (*digit) != 0) temp_circuit_data[*select] = pow(10, *digit) * temp_circuit_data[*select];
        else if (data == 0x16) temp_circuit_data[*select] = pow(10, *digit) * temp_circuit_data[*select] + 1;
        else if (data == 0x1E) temp_circuit_data[*select] = pow(10, *digit) * temp_circuit_data[*select] + 2;
        else if (data == 0x26) temp_circuit_data[*select] = pow(10, *digit) * temp_circuit_data[*select] + 3;
        else if (data == 0x25) temp_circuit_data[*select] = pow(10, *digit) * temp_circuit_data[*select] + 4;
        else if (data == 0x2E) temp_circuit_data[*select] = pow(10, *digit) * temp_circuit_data[*select] + 5;
        else if (data == 0x36) temp_circuit_data[*select] = pow(10, *digit) * temp_circuit_data[*select] + 6;
        else if (data == 0x3D) temp_circuit_data[*select] = pow(10, *digit) * temp_circuit_data[*select] + 7;
        else if (data == 0x3E) temp_circuit_data[*select] = pow(10, *digit) * temp_circuit_data[*select] + 8;
        else if (data == 0x46) temp_circuit_data[*select] = pow(10, *digit) * temp_circuit_data[*select] + 9;

        else if (data == 0x66 && (*digit) != 0){
            temp_circuit_data[*select] = round(temp_circuit_data[*select]/pow(10, (*digit)));
            *digit--;
        }

        else if (data == 0x5A){
            *circuit_data = *temp_circuit_data;
            *digit=0;
        }

        type_ready = false;
    } else if (!type_ready){
        if (data == 0xF045 ||
            data == 0xF016 ||
            data == 0xF01E ||
            data == 0xF026 ||
            data == 0xF025 ||
            data == 0xF02E ||
            data == 0xF036 ||
            data == 0xF03D ||
            data == 0xF03E ||
            data == 0xF046 ||
            data == 0xF066 ||
            data == 0xF05A){
        	*type_ready = true;
            *digit ++;
        }
    }
}

void compute(int size,
            float *Vs,
            float *Ic,
            float *Vc,
            float *v_stored,
            float amp,
            float freq,
            float phase,
            float cap,
            float res,
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
        *v_stored = Vc[size];
    }

    else if (sw1 && !sw2)
    {
        Ic[size] = cap * amp * freq * cos(arg);
        Vc[size] = Vs[size] * (1 - exp( -(t-t_not) / (res * cap) ) );
        *v_stored = Vc[size];
    }

    else if (sw1 && sw2)
    {
        Ic[size] = ((amp * sin(arg) * exp( -(t-t_not) / (res * cap) ) ) / res) + (amp * cap * freq * (1 - exp( -(t-t_not) / (res * cap) ) ) * cos(arg));
        Vc[size] = Vs[size] * (1 - exp( -(t-t_not) / (res * cap) ) );
        *v_stored = Vc[size];
    }

    else if (!sw1 && sw2)
    {
        Vc[size] = *(v_stored) *  exp( -(t-t_not) / (res * cap) );
        Ic[size] = - Vc[size] / res;
    }

}
