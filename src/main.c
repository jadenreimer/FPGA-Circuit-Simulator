#include <stdbool.h>
#include <math.h>
<<<<<<< HEAD
#include "arm.h"
=======
>>>>>>> 68166b262c0ebf7be46a028a885996fe81a2909c

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
#define CHAR_BUFFER 0xC9000000

//VGA
#define X_DIM 320
#define Y_DIM 240
#define GRAPH_LEN 50
#define ARROW_LEN (GRAPH_LEN/20)

void clear_screen();

volatile int pixel_buffer_start; // global variable

void draw_line(int xi, int yi, int xf, int yf, short int line_color);
// void draw_image(int x_start, int y_start, int x_size, int y_size, extern short image);
void draw_graph(int x, int y, int size, float values[size], short int colour);
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
<<<<<<< HEAD
void change_data( int *select, int *digit, bool *type_ready, float *circuit_data, float *temp_circuit_data);;
int get_jtag( void );
=======
void change_data( int *select, int *digit, bool *type_ready, float *circuit_data, float *temp_circuit_data);
void draw_frame(int x0, int y0, int x1, int y1, short int colour);
void draw_rect(int x0, int y0, int x1, int y1, short int colour);
void draw_circle(int x, int y, int r, short int colour);
void draw_switches(int len, int x0, int y0, int x1, int y1, bool sw1, bool sw2, int colour);
void draw_circuit(int x, int y, short int colour, bool sw1, bool sw2);
void write_char(int x, int y, char c);
void write_string(int x, int y, int size, char string[size]);
void clear_chars();
>>>>>>> 68166b262c0ebf7be46a028a885996fe81a2909c

int main(void){
    bool sw1 = true;
    bool sw1_ready = true;
    bool sw2 = false;
    bool sw2_ready = true;

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
    clear_chars();

    /* set back pixel buffer to start of SDRAM memory */
    *(pixel_ctrl_ptr + 1) = SDRAM_MEM;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer

    int size = 70;
    int debug_switches = 0;

    //Graph data
    float Vs[size];// = {0};
    float Ic[size];// = {0};
    float Vc[size];// = {0};

    for(int i = 0; i<size; i++){
        Vs[i] = 0;
        Vc[i] = 0;
        Ic[i] = 0;
    }

    float v_stored = 0.0;
    float test[30] = {Vs[69], Ic[69], Vc[69], v_stored, 3, 1, 2, 4, 8, 3, 1, 2, 4, 8, 3, 1, 2, 4, 8, 3, 1, 2, 4, 8, 3, 1, 1, 2, 1, 1};

    //time data
    int tc = 0;
    double t = 0.0;
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

    int lit = 0;

    while (true){
        //  clear screen
        clear_screen();
        //size of 31043
        // for (int y = 0; y < 54; y++) {
        //     for (int x = 0; x < 320; x++) {
        //         plot_pixel(x, y, circuit[y][x]);
        //     }
        // }
        clear_chars();

        draw_circuit(40, 100, WHITE, sw1, sw2);

        // draw_switches(50, 50, 50, 180, 60, false, true, RED);

        // char string[] = "testing 100pF 10kOhm";
        // write_string(40+lit, 5, 20, string);
        // lit++;

        //Calculate Ic and Vs
        if(tc == 1){

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

            //Debugging only
            for(int i=0; i<29; i++){
                // if(i < 15)
                //     test[i] = sin(i)*test[i+1];
                // else test[i] = sin(i)/test[i+1];
                test[i] = test[i+1];
            }
            test[29] = sin(t);
            tc = 0;
        }

		if(debug_switches == 8){
            sw1=false;
            sw2=true;
        }

        //Draw graphs to the right of the circuit
        draw_graph(240, 60, sizeof(Vs)/sizeof(Vs[0]), Vs, WHITE);//some fucking difference between Vs and test here means the graph straight up doesn't plot vs plotting
        draw_graph(240, 180, sizeof(Vc)/sizeof(Vc[0]), Vc, WHITE);//some fucking difference between Vs and test here means the graph straight up doesn't plot vs plotting
        // draw_graph(240, 180, sizeof(Ic)/sizeof(Ic[0]), Ic, WHITE);//some fucking difference between Vs and test here means the graph straight up doesn't plot vs plotting
        tc++;
        debug_switches++;
        // draw_graph(graph_x_dist, graph_y_dist + GRAPH_LEN + 20); //this one is drawn below the other
        //draw_graph(graph_x_dist, graph_y_dist);
        //draw_graph(graph_x_dist, graph_y_dist + GRAPH_LEN + 20); //this one is drawn below the other
        // int i, j;
        // for (i=0; i<120; i++)
        //     for (j=0; j<80; j++)

        // int sw1_old = sw1;
        // int sw2_old = sw2;
        // set_switches(&sw1, &sw2, &sw1_ready, &sw2_ready);
        // if (sw1_old != sw1 || sw2_old != sw2) t_not = t;

        // tab_over(select, tab_ready);
        // change_data(select, type_ready, circuit_data, temp_circuit_data);

        //  wait for sync
        wait_for_vsync(); // swap front and back buffers on VGA vertical sync
        pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
    }
}

void draw_circuit(int x, int y, short int colour, bool sw1, bool sw2){

    int width = 5;
    int ctr = 80;
    //source
    int radius = 30;
    draw_circle(x, y, radius, WHITE);
    draw_circle(x, y, radius-width, BLACK);
    //plus
    draw_rect(x-3, y-20, x+3, y, RED);
    draw_rect(x-14, y-13, x+14, y-7, RED);
    //minus
    draw_rect(x-14, y+10, x+14, y+16, BLUE);

    //capacitor
    int cap_x = (x+ctr -5*width);
    draw_rect(cap_x, y-2*width, cap_x + 10*width, y-width, WHITE);
    draw_rect(cap_x, y+width, cap_x + 10*width, y+2*width, WHITE);

    //resistor
    int diag_x = 15;
    int diag_y = 10;
    int res_x = (x+2*ctr);
    int res_y = y-diag_x;//resistor goes down 40 pixels from here

    for(int dec = 0; dec<5; dec++){//make the resistor lines thicker
        draw_line(res_x, res_y, res_x-diag_x, res_y+diag_y, WHITE);//from the line out: x- y+
        draw_line(res_x-diag_x, res_y+diag_y, res_x+diag_x, res_y+2*diag_y, WHITE);//x+ y+
        draw_line(res_x+diag_x, res_y+2*diag_y, res_x-diag_x, res_y+3*diag_y, WHITE);//x- y+
        draw_line(res_x-diag_x, res_y+3*diag_y, res_x+diag_x, res_y+4*diag_y, WHITE);//x+ y+
        draw_line(res_x+diag_x, res_y+4*diag_y, res_x, res_y+5*diag_y, WHITE);//back into the line: x- y+
        dec++;
        res_y++;
    }
    //switches
    draw_switches(30, x + 10, y-ctr, x + 10 + ctr, y - ctr, sw1, sw2, WHITE);

    //ground
    int gnd_ctr = x + ctr;
    int gnd_y = y+ctr-20;
    draw_rect(gnd_ctr-width/2, gnd_y, gnd_ctr+width/2, gnd_y+2*width, WHITE);//stem
    draw_rect(gnd_ctr-4*width, gnd_y+2*width, gnd_ctr+4*width, gnd_y+3*width, WHITE);//1st layer
    draw_rect(gnd_ctr-2*width, gnd_y+4*width, gnd_ctr+2*width, gnd_y+5*width, WHITE);//2nd
    draw_rect(gnd_ctr-width, gnd_y+6*width, gnd_ctr+width, gnd_y+7*width, WHITE);//3rd

    //wires

    //text add-ons
}

void clear_screen(){
    short int line_colour = BLACK;
    for (int x = 0; x < X_DIM; x++ ){
    	for (int y = 0; y < Y_DIM ; y++ ){
        	plot_pixel(x, y, line_colour);
        }
    }
    plot_pixel(5, 5, RED);
    return;
}

void draw_graph(int x, int y, int size, float values[size], short int colour){
    //bars of the graph
    draw_line(x, y + GRAPH_LEN, x, y - GRAPH_LEN, colour);//up-down
    draw_line(x, y, x + GRAPH_LEN, y, colour);//horizontal

    //arrows of the graph
    //top arrow
    draw_line(x, y - GRAPH_LEN, x-ARROW_LEN, y-GRAPH_LEN+ARROW_LEN, colour);
    draw_line(x, y - GRAPH_LEN, x+ARROW_LEN, y-GRAPH_LEN+ARROW_LEN, colour);
    //right arrow
    draw_line(x + GRAPH_LEN, y, x+GRAPH_LEN-ARROW_LEN, y-ARROW_LEN, colour);
    draw_line(x + GRAPH_LEN, y, x+GRAPH_LEN-ARROW_LEN, y+ARROW_LEN, colour);
    //bottom arrow
    draw_line(x, y + GRAPH_LEN, x-ARROW_LEN, y+GRAPH_LEN-ARROW_LEN, colour);
    draw_line(x, y + GRAPH_LEN, x+ARROW_LEN, y+GRAPH_LEN-ARROW_LEN, colour);

    float max = 0.000001;
    //find the max in the array
    for(int i = 0; i < size-1; i++){
        float check = values[i];
        if(check < 0) check = -check;
        if(max < check) max = check;
    }

    //graph the values
    for(int i = 0; i < size-2; i++){
        draw_line((int)(x+i*(GRAPH_LEN-ARROW_LEN)/size), (int)(y-(GRAPH_LEN-ARROW_LEN)*(values[i]/max)),
                (int)(x+(i+1)*(GRAPH_LEN-ARROW_LEN)/size), (int)(y-(GRAPH_LEN-ARROW_LEN)*(values[i+1]/max)), RED);
    }
}

void write_char(int x, int y, char c) {
  // VGA character buffer
  volatile char * character_buffer = (char *) (0xC9000000 + (y<<7) + x);
  *character_buffer = c;
}

void write_string(int x, int y, int size, char string[size]){
    if(y<60){
        for (int i = 0; i < size; i++) {
            if((x+i)<79)
                write_char(x + i, y, string[i]);
        }
    }
}

void clear_chars(){
    for (int x = 0; x < 79; x++)
        for(int y = 0; y < 59; y++)
            write_char(x, y, '\0');
}

void draw_switches(int len, int x0, int y0, int x1, int y1, bool sw1, bool sw2, int colour){
    if(sw1){
        draw_line(x0, y0, x0 + len, y0, colour);
    }else{
        draw_line(x0, y0, (x0 + len)*4/5, y0 + len/3, colour);
    }

    if(sw2){
        draw_line(x1, y1, x1 + len, y1, colour);
    }else{
        draw_line(x1, y1, (x1 + len)*4/5, y1 + len/3, colour);
    }

    //circle bois 1
    int radius = 5;
    int width = 2;
    draw_circle(x0, y0, radius, colour);//left
    draw_circle(x0, y0, radius-width, BLACK);
    draw_circle(x0+len, y0, radius, colour);//right
    draw_circle(x0+len, y0, radius-width, BLACK);

    //circle bois 2
    draw_circle(x1, y1, radius, colour);//left
    draw_circle(x1, y1, radius-width, BLACK);
    draw_circle(x0+len, y0, radius, colour);//right
    draw_circle(x0+len, y0, radius-width, BLACK);
}

void draw_frame(int x0, int y0, int x1, int y1, short int colour){
    draw_line(x0, y0, x0, y1, colour);//down
    draw_line(x0, y1, x1, y1, colour);//right
    draw_line(x1, y1, x1, y0, colour);//up
    draw_line(x1, y0, x0, y0, colour);//left
}

void draw_rect(int x0, int y0, int x1, int y1, short int colour){
    for(int x = x0; x<=x1; x++){
        for (int y = y0; y<=y1; y++) {
            plot_pixel(x, y, colour);
        }
    }
}

void draw_circle(int x, int y, int r, short int colour){
    for(int deg=0; deg<360; deg++){
        draw_line(x, y, x + r*cos(deg), y + r*sin(deg), colour);
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
    if(x<X_DIM && x>=0 && y<Y_DIM && y>=0)
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

void set_switches( bool *sw1, bool *sw2, bool *sw1_ready, bool *sw2_ready )
{
    int data = get_jtag();

    if (data == 0x1A && (*sw1_ready)){
        *sw1 = !(*sw1);
        // *sw1_ready = false;
    }

    // else if (data == 0xF01A && !(*sw1_ready)){
    //     *sw1_ready = true;
    // }

    if (data == 0x22 && (*sw2_ready)){
        *sw2 = !(*sw2);
        // *sw2_ready = false;
    }

    // else if (data == 0xF022 && !(*sw2_ready)){
    //     *sw2_ready = true;
    // }
}

void tab_over( int *select, int *digit, bool *tab_ready, float *circuit_data, float *temp_circuit_data)
{
    int data = get_jtag();

    if (data == 0x0D && (*tab_ready)){
        (*select)++;
        if ((*select)>4) (*select) = 0;
        // *tab_ready = false;
        memcpy(temp_circuit_data, circuit_data, 6);
    }

    // else if (data == 0xF00D && !(*tab_ready)){
    //     *tab_ready = true;
    // }

}

void change_data( int *select, int *digit, bool *type_ready, float *circuit_data, float *temp_circuit_data)
{
    int data = get_jtag();

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

        //type_ready = false;
    }// else if (!type_ready){
    //     if (data == 0xF045 ||
    //         data == 0xF016 ||
    //         data == 0xF01E ||
    //         data == 0xF026 ||
    //         data == 0xF025 ||
    //         data == 0xF02E ||
    //         data == 0xF036 ||
    //         data == 0xF03D ||
    //         data == 0xF03E ||
    //         data == 0xF046 ||
    //         data == 0xF066 ||
    //         data == 0xF05A){
    //     	*type_ready = true;
    //         *digit ++;
    //     }
    // }
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
            bool sw2){

    for (int i = 0; i<size; i++){
        Ic[i] = Ic[i+1];
        Vs[i] = Vs[i+1];
        Vc[i] = Vc[i+1];
    }

    float arg = (freq * (t - t_not)) - phase;

    Vs[size-1] = amp * sin(arg);

    if (!sw1 && !sw2){
        Ic[size-1] = 0;
        Vc[size-1] = Vc[size-1];//Vs[size-1];
        *v_stored = Vc[size-1];
    }

    else if (sw1 && !sw2){
        Ic[size-1] = cap * amp * freq * cos(arg);
        Vc[size-1] = Vs[size-1] * (1 - exp( -(t-t_not) / (res * cap) ) );
        *v_stored = Vc[size-1];
    }

    else if (sw1 && sw2){
        Ic[size-1] = ((amp * sin(arg) * exp( -(t-t_not) / (res * cap) ) ) / res) + (amp * cap * freq * (1 - exp( -(t-t_not) / (res * cap) ) ) * cos(arg));
        Vc[size-1] = Vs[size-1] * (1 - exp( -(t-t_not) / (res * cap) ) );
        *v_stored = Vc[size-1];
    }

    else if (!sw1 && sw2){
        Vc[size-1] = *(v_stored) *  exp( -(t-t_not) / (res * cap) );
        Ic[size-1] = - Vc[size-1] / res;
    }
}

int get_jtag( void ){
    //read from PS/2 address
    volatile int* PS2_ptr = (int*) 0xFF201000;
    //collect its data
    int PS2_data = *(PS2_ptr);
    //if RVALID is 1
    //send back the make code of the PS/2 data
    if (PS2_data & 0x00008000) return (data & 0xFF);
    //otherwise, send NULL
    else return ('\0');
}
