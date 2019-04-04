#include <stdbool.h>
#include <math.h>
//#include "arm.h"

// Colours
#define RED 0xF800
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

        tab_over(&select, &digit, &tab_ready, circuit_data, temp_circuit_data);
        change_data(&select, &digit, &type_ready, circuit_data, temp_circuit_data);
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
        memcpy(temp_circuit_data, circuit_data, 6);
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
            memcpy(circuit_data, temp_circuit_data, 6);
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
