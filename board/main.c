/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

///////////////////////////////////////////////////////////////////////////////
//  Includes
///////////////////////////////////////////////////////////////////////////////
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "board.h"
#include "rdc_semaphore.h"
#include "debug_console_imx.h"
#include "gpio_pins.h"
#include "gpio_imx.h"
#include "i2c_xfer.h"
#include "fxas21002.h"
#include "fxos8700.h"
#include "mpl3115.h"
#include "uart_imx.h" 


//#include "gpio_wrapper.h" // Added By Rohan Tabish


static void UART_SendDataPolling(UART_Type *base, const uint8_t *txBuff, uint32_t txSize);
static void UART_ReceiveDataPolling(UART_Type *base, uint8_t *rxBuff, uint32_t rxSize);

////////////////////////////////////////////////////////////////////////////////
// Static Variable
////////////////////////////////////////////////////////////////////////////////

//static SemaphoreHandle_t xSemaphore;


////////////////////////////////////////////////////////////////////////////////
// Quanser Code Starts Here
////////////////////////////////////////////////////////////////////////////////

#define DEBUG 			0
#define RECORD 			0
#define HX_SIZE 		52

#define SIM_STEP      	0.01
#define PERIOD        	0.05
#define RESTART_TIME  	0.3

#define MAX_VOLTAGE		3

    // Setup UART init structure.
uart_init_config_t initConfig = {
    .baudRate   = 115200u,
    .wordLength = uartWordLength8Bits,
    .stopBitNum = uartStopBitNumOne,
    .parity     = uartParityDisable,
    .direction  = uartDirectionTxRx
};

const uint8_t bufferData1[] = "\n\r TX\RX\n\r";

uint8_t rxChar = 0;
//uint32_t byteCount = 0;


double P[10] = {-.500, -2.4000, 0.00, 0.1200, 0.1200, -2.5000, -0.0200, 0.200, 2.1000, 10.0000};

double Hx[HX_SIZE][6] = {

        {-0.948683, -0.316228, 0, 0.000000,  0.000000,  0},
        {1.000000,  0.000000,  0, 0.000000,  0.000000,  0},
        {0.000000,  1.000000,  0, 0.000000,  0.000000,  0},
        {0.000000,  0.000000,  0, 1.000000,  0.000000,  0},
        {0.000000,  0.000000,  0, 0.000000,  1.000000,  0},
        {-0.948683, 0.316228,  0, 0.000000,  0.000000,  0},
        {-1.000000, 0.000000,  0, 0.000000,  0.000000,  0},
        {0.000000,  -1.000000, 0, 0.000000,  0.000000,  0},
        {0.000000,  0.000000,  0, -1.000000, 0.000000,  0},
        {0.000000,  0.000000,  0, 0.000000,  -1.000000, 0},
        {0.000000,  0.980581,  0, 0.000000,  0.196116,  0},
        {0.976164,  0.092968,  0, 0.195233,  0.018594,  0},
        {0.000000,  -0.980581, 0, 0.000000,  -0.196116, 0},
        {-0.975088, 0.038239,  0, -0.217961, 0.015296,  0},
        {-0.976164, 0.092968,  0, -0.195233, 0.018594,  0},
        {-0.930261, 0.310087,  0, -0.186052, 0.062017,  0},
        {0.980030,  -0.031112, 0, 0.196006,  -0.012445, 0},
        {-0.975088, -0.038239, 0, -0.217961, -0.015296, 0},
        {-0.884112, 0.160748,  0, -0.434019, 0.064299,  0},
        {0.896258,  -0.256074, 0, 0.358503,  -0.051215, 0},
        {0.980581,  0.000000,  0, 0.196116,  0.000000,  0},
        {0.980030,  0.031112,  0, 0.196006,  0.012445,  0},
        {-0.920002, 0.036079,  0, -0.389648, 0.021647,  0},
        {-0.980581, 0.000000,  0, -0.196116, 0.000000,  0},
        {-0.927490, -0.206109, 0, -0.309163, -0.041222, 0},
        {-0.945330, -0.171878, 0, -0.275005, -0.034376, 0},
        {-0.924294, 0.088028,  0, -0.369718, 0.035211,  0},
        {-0.880102, 0.117347,  0, -0.457653, 0.046939,  0},
        {-0.945330, 0.171878,  0, -0.275005, 0.034376,  0},
        {-0.927490, 0.206109,  0, -0.309163, 0.041222,  0},
        {-0.931312, 0.310437,  0, -0.186262, 0.039913,  0},
        {0.927929,  0.029458,  0, 0.371171,  0.017675,  0},
        {0.928477,  0.000000,  0, 0.371391,  0.000000,  0},
        {0.896258,  0.256074,  0, 0.358503,  0.051215,  0},
        {0.791602,  -0.376953, 0, 0.474961,  -0.075391, 0},
        {0.791602,  0.376953,  0, 0.474961,  0.075391,  0},
        {0.927929,  -0.029458, 0, 0.371171,  -0.017675, 0},
        {0.976164,  -0.092968, 0, 0.195233,  -0.018594, 0},
        {-0.928477, 0.000000,  0, -0.371391, 0.000000,  0},
        {-0.920002, -0.036079, 0, -0.389648, -0.021647, 0},
        {-0.884112, -0.160748, 0, -0.434019, -0.064299, 0},
        {-0.880102, -0.117347, 0, -0.457653, -0.046939, 0},
        {-0.930261, -0.310087, 0, -0.186052, -0.062017, 0},
        {-0.976164, -0.092968, 0, -0.195233, -0.018594, 0},
        {-0.931312, -0.310437, 0, -0.186262, -0.039913, 0},
        {-0.924294, -0.088028, 0, -0.369718, -0.035211, 0},
        {-0.853630, -0.081298, 0, -0.512178, -0.048779, 0},
        {0.853630,  0.081298,  0, 0.512178,  0.048779,  0},
        {0.924294,  0.088028,  0, 0.369718,  0.035211,  0},
        {-0.853630, 0.081298,  0, -0.512178, 0.048779,  0},
        {0.853630,  -0.081298, 0, 0.512178,  -0.048779, 0},
        {0.924294,  -0.088028, 0, 0.369718,  -0.035211, 0}


};

double hx[HX_SIZE][1] = {
        {0.284605},
        {0.200000},
        {1.200000},
        {0.400000},
        {1.300000},
        {0.284605},
        {0.200000},
        {1.200000},
        {0.400000},
        {1.300000},
        {1.382619},
        {0.326318},
        {1.382619},
        {0.238610},
        {0.242646},
        {0.344196},
        {0.259475},
        {0.238610},
        {0.368112},
        {0.558241},
        {0.215728},
        {0.259475},
        {0.284660},
        {0.215728},
        {0.315347},
        {0.285318},
        {0.285211},
        {0.334439},
        {0.285318},
        {0.315347},
        {0.326846},
        {0.301356},
        {0.259973},
        {0.558241},
        {0.753153},
        {0.753153},
        {0.301356},
        {0.326318},
        {0.259973},
        {0.284660},
        {0.368112},
        {0.334439},
        {0.344196},
        {0.242646},
        {0.326846},
        {0.285211},
        {0.348769},
        {0.421937},
        {0.364436},
        {0.348769},
        {0.421937},
        {0.364436},

};


struct controller_storage {
    double int_elevation;
    double int_pitch;
    double int_travel;

    double elevation1;
    double pitch1;
    double travel1;

    double elevation2;
    double pitch2;
    double travel2;
} controller_storage;


struct state {
    double elevation;
    double pitch;
    double travel;
    double d_elevation;
    double d_pitch;
    double d_travel;
    int safe;
} state;

struct command {
    double u1;
    double u2;
} command;




double vol_right = 0;
double vol_left = 0;


double add_left = 0;
double add_right = 0;

double C[HX_SIZE][1];

int32_t getOneDecimalPlace(float num)
{
    if (num < 0)
        return -(((int32_t)(num * 10)) % 10);
    else
        return (((int32_t)(num * 10)) % 10);
}

double factorial(double x)
{
    double result = 1;
    int i;
    for (i=1;i<=x;i++)
        result *= i;
    return result;
}

double pow(double x,double y)
{
    double result = 1;
    int i;
    for (i=0;i<y;i++)
        result *= x;
    return result;
}

double sini(double x)
{
    double n = 10;
    double sine = x;
    int isNeg;
    isNeg = 1;
    double i;
    for (i=3;i<=n;i+=2)
    {
        if(isNeg == 1)
        {
            sine -= pow(x,i)/factorial(i);
            isNeg = 0;
        }
        else
        {
            sine += pow(x,i)/factorial(i);
            isNeg = 1;
        }
    }
    return sine;
}

double cosi(double x)
{
    return 1 - pow(sini(x),2);
}


void matrix_mult(double A[HX_SIZE][6], double B[6][1], int m, int n, int k) {

    int r = 0;
    int c = 0;
    int kk = 0;
    for (r = 0; r < m; r++) {
        for (c = 0; c < n; c++) {
            C[r][c] = 0;
            for (kk = 0; kk < k; kk++) {
                C[r][c] += A[r][kk] * B[kk][c];
            }
        }
    }

}

double voltage_max_min(double voltage) {

    if (voltage > MAX_VOLTAGE)
        voltage = MAX_VOLTAGE;
    else if (voltage < -MAX_VOLTAGE)
        voltage = -MAX_VOLTAGE;
    return voltage;
}


struct state eval_state(struct state state_x, struct command U) {

    

    struct state d_state;



    d_state.elevation = state_x.d_elevation;
    d_state.pitch = state_x.d_pitch;
    d_state.travel = state_x.d_travel;
    d_state.d_elevation = P[0] * cosi(state_x.elevation) + P[1] * sini(state_x.elevation) + P[2] * state_x.d_travel + P[7] * cosi(state_x.pitch) * (U.u1 + U.u2);

    d_state.d_pitch =  P[4] * sini(state_x.pitch) + P[3] * cosi(state_x.pitch) + P[5] * state_x.d_pitch + P[8] * (U.u1 - U.u2);
    
    d_state.d_travel = P[6] * state_x.d_travel + P[9] * sini(state_x.pitch) * (U.u1 + U.u2);

    state_x.elevation += SIM_STEP * d_state.elevation;
    state_x.pitch += SIM_STEP * d_state.pitch;
    state_x.travel += SIM_STEP * d_state.travel;
    state_x.d_elevation += SIM_STEP * d_state.d_elevation;
    state_x.d_pitch += SIM_STEP * d_state.d_pitch;
    state_x.d_travel += SIM_STEP * d_state.d_travel;

    

    return state_x;

}

struct command controller_safety(struct state x, struct controller_storage *cs) {

    struct command U;

  

    cs->int_travel += x.travel;
    cs->int_pitch += x.pitch;
    cs->int_elevation += x.elevation;

    U.u1 = + -6.5 * x.elevation - .701 * x.pitch - 45.7161 * PERIOD * x.d_elevation -
           3.051 * PERIOD * x.d_pitch; //-0.0333*cs->int_elevation -0.001*cs->int_pitch;
    U.u2 = + -6.5 * x.elevation + .5701 * x.pitch - 45.7529 * PERIOD * x.d_elevation +
           5.970 * PERIOD * x.d_pitch; //-0.03*cs->int_elevation +0.001*cs->int_pitch;



    cs->elevation2 = cs->elevation1;
    cs->elevation1 = x.elevation;

    cs->pitch2 = cs->pitch1;
    cs->pitch1 = x.pitch;

    cs->travel2 = cs->travel1;
    cs->travel1 = x.travel;


    U.u1 = voltage_max_min(U.u1);
    U.u2 = voltage_max_min(U.u2);



    return U;
}

struct command controller_complex(struct state x, struct controller_storage *cs) {
    struct command U;
  

    cs->int_travel += x.travel;
    cs->int_pitch += x.pitch;
    cs->int_elevation += x.elevation;


    //  U.u1 = 1+ -6.5 * x.elevation  - 3.01 * (x.pitch + 0.08)  -25.7161 * x.d_elevation ;//-3.051 * x.d_pitch; //-.0333*cs->int_elevation -0.001*cs->int_pitch;
    //  U.u2  = 1 + -6.5 * x.elevation  + 5.5701 * (x.pitch + 0.08) -25.7529 * x.d_elevation; // +5.970 * x.d_pitch; //-0.03*cs->int_elevation +0.001*cs->int_pitch;

    U.u1 = -.5 * x.elevation - .701 * (x.pitch - 0.2) - 45.7161 * PERIOD * x.d_elevation - 3.051 * PERIOD * x.d_pitch +
           0.2 * x.travel; //-0.0333*cs->int_elevation -0.001*cs->int_pitch;
    U.u2 = -.5 * x.elevation + .5701 * (x.pitch - 0.2) - 45.7529 * PERIOD * x.d_elevation + 5.970 * PERIOD * x.d_pitch -
           0.2 * x.travel; //-0.03*cs->int_elevation +0.001*cs->int_pitch;

    cs->elevation2 = cs->elevation1;
    cs->elevation1 = x.elevation;

    cs->pitch2 = cs->pitch1;
    cs->pitch1 = x.pitch;

    cs->travel2 = cs->travel1;
    cs->travel1 = x.travel;

    U.u1 += add_right;
    U.u2 += add_left;


    U.u1 = voltage_max_min(U.u1);
    U.u2 = voltage_max_min(U.u2);

    return U;
}


int check_safety(struct state x) {

    double X[6][1] = {{x.elevation},
                      {x.pitch},
                      {x.travel},
                      {x.d_elevation},
                      {x.d_pitch},
                      {x.d_travel}};

    matrix_mult(Hx, X, HX_SIZE, 1, 6);

    int all_small = 1;
    int k = 0;
    for (k = 0; k < HX_SIZE; k++) {
        if (C[k][0] > hx[k][0]) {

            all_small = 0;

            break;
        }

    }

    //  if (x.elevation+0.333*x.pitch > -0.3 && x.elevation-0.333*x.pitch>-0.3  && x.elevation < 0.35 && x.d_elevation > -0.3 && x.d_elevation < 0.4  && x.d_pitch > -1.3 && x.d_pitch < 1.3)
    if (all_small == 1)
        return 1;
    else
        return 0;
}


struct state simulate_fixed_control(struct state init_state, struct command U, double time) {

    struct state state_x;

    state_x = init_state;

    int steps = time / SIM_STEP;
    int k = 0;
    for (k = 0; k < steps; k++) {

        state_x = eval_state(state_x, U);
        
        if (check_safety(state_x) == 0) {
            state_x.safe = 0;

            return state_x;
        }
    }
    state_x.safe = 1;
    

    return state_x;
}

struct state simulate_with_controller(struct state init_state, double time) {

	struct state state_x;

	state_x = init_state;

    int steps = time / SIM_STEP;

    struct controller_storage cs;
    cs.int_elevation = 0;
    cs.int_pitch = 0;
    cs.int_travel = 0;

    int k = 0;

    for (k = 0; k < steps; k++) {
        struct command U = controller_safety(state_x, &cs);
        state_x = eval_state(state_x, U);
        if (check_safety(state_x) == 0) {

            state_x.safe = 0;

           return state_x;

    }

    state_x.safe = 1;
    }

    return state_x;

}


//The outcome determines weather the safety controller should be used or not
int decide(struct state current_state, struct command U, double time) {
    //struct state x2 = simulate_fixed_control(current_state, U, time);
    struct state x2 = simulate_fixed_control(current_state, U, RESTART_TIME);
	// struct state x10;
    struct state x10 = simulate_with_controller(x2, 1);


    if (x2.safe == 1 && x10.safe == 1)
        return 1;
    else
        return 0;

}

void write_to_serial(double double_volts[]){
    // TODO: here write the values of the vol_right and vol_left to the serial port. 8 bytes
    int voltages[2];
    unsigned char Txbuffer[10];
    char i = 0;
    Txbuffer[0] = 0xCC; // End Byte
    Txbuffer[9] = 0xFF; // Start Byte




    voltages[0] = (int) (double_volts[0] * 10000.0);
    voltages[1] = (int) (double_volts[1] * 10000.0);
  


//    printf("voltages [0] = %d \r\n", voltages[0]);
//    printf("voltages [1] = %d \r\n", voltages[1]);

    for(i = 0; i < 2;i++ ){


         Txbuffer[4*i+1]            = (char) (voltages[i] & 0xff); /* first byte */
         Txbuffer[4*i + 2]          = (char) (voltages[i] >> 8  & 0xff); /* second byte */
         Txbuffer[4*i + 3]          = (char) (voltages[i] >> 16 & 0xff); /* third byte */
         Txbuffer[4*i + 4]          = (char) (voltages[i] >> 24 & 0xff); /* fourth byte */
    
    }


    UART_SendDataPolling(BOARD_DEBUG_UART_BASEADDR, Txbuffer, 10);


    return;
}

void read_from_serial(int* sensor_readings){


    //TODO: Here read the values of the sensors from serial 12 bytes
    char rxChar1[13];
    double tmpSend[2];


    while (1){

        UART_ReceiveDataPolling(BOARD_DEBUG_UART_BASEADDR, &rxChar, 1);


        if(rxChar == 0xAA){
        	rxChar = 0xBB;

        	UART_SendDataPolling(BOARD_DEBUG_UART_BASEADDR, &rxChar, 1);
            UART_ReceiveDataPolling(BOARD_DEBUG_UART_BASEADDR, rxChar1, 13);

            sensor_readings[0] = *(unsigned int *)&rxChar1[0];
            sensor_readings[1] = *(unsigned int *)&rxChar1[4];
            sensor_readings[2] = *(unsigned int *)&rxChar1[8];

            tmpSend[0] = (double)sensor_readings[0];
            tmpSend[1] = (double)sensor_readings[1];


            //write_to_serial(tmpSend);

            break;
        }
   }

    return;
}



void MainTask(void *pvParameters)
{
    
        /* .. Writing the control values to the left and write motor......*/
    //printf("Main task is called \r\n");


    int step = 0;
    double vol_step = 0.1;
    int byteCount;
    //
    //    FILE *ofp;
    //    ofp = fopen("recorded_data.txt", "w");

    //    int sensor_readings[3];
    //
    //    err = ioctl(File_Descriptor, Q8_ENC, sensor_readings);
    //    if (err != 0) {
    //        perror("Epic Fail first enc read\n");
    //        return -1;
    //    }

    int sensor_readings[3];

    read_from_serial(sensor_readings);

    int base_travel = sensor_readings[0];
    int base_pitch = sensor_readings[1];
    int base_elevation = sensor_readings[2];


    double d_travel = 0;
    double d_pitch = 0;
    double d_elevation = 0;

    struct controller_storage storage_safety;
    storage_safety.int_travel = 0;
    storage_safety.int_pitch = 0;
    storage_safety.int_elevation = 0;

    struct controller_storage storage_complex;
    storage_complex.int_travel = 0;
    storage_complex.int_pitch = 0;
    storage_complex.int_elevation = 0;

    struct controller_storage storage; //for the current loop

    int remaining_safety_cycles = 0;

    double voltages[2];

	byteCount = sizeof(bufferData1);
    UART_SendDataPolling(BOARD_DEBUG_UART_BASEADDR, bufferData1, byteCount);
   // printf("Main Task has been triggerred..... \r\n");

    vTaskDelay(100);

    while(1){

        for (step = 0; step < 15000; step++) {

        //        unsigned short int tmparray[4];
        //        tmparray[0] = Q8_dacVTO((vol_right), 1, 10);
        //        tmparray[1] = Q8_dacVTO((vol_left), 1, 10);
        //        ioctl(File_Descriptor, Q8_WR_DAC, tmparray);

            
            voltages[0] = vol_left;
            voltages[1] = vol_right;
            write_to_serial(voltages);

            /* .. Reading the Encoder values from the helicopter......*/
            read_from_serial(sensor_readings);
        //        err = ioctl(File_Descriptor, Q8_ENC, sensor_readings);
        //        if (err != 0) {
        //            perror("Epic Fail first enc read\n");
        //            return -1;
        //        }

            int travel = sensor_readings[0] - base_travel;
            int pitch = sensor_readings[1] - base_pitch;
            int elevation = -(sensor_readings[2] - base_elevation) - 300;

            struct state cs;

            cs.elevation = 1.0 / 10.0 * 3.1415 / 180.0 * (double) elevation;
            cs.pitch = 90.0 / 1000.0 * 3.1415 / 180.0 * (double) pitch;
            cs.travel = 3.1415 / 4089.00 * (double) travel;

            cs.d_travel = (cs.travel - storage.travel1) / PERIOD;
            cs.d_pitch = (cs.pitch - storage.pitch1) / PERIOD;
            cs.d_elevation = (cs.elevation - storage.elevation1) / PERIOD;

            storage.elevation2 = storage.elevation1;
            storage.elevation1 = cs.elevation;

            storage.pitch2 = storage.pitch1;
            storage.pitch1 = cs.pitch;

            storage.travel2 = storage.travel1;
            storage.travel1 = cs.travel;


            if (step < 50) {
                vol_right = 0;
                vol_left = 0;
            } 
            else {
                
                struct command U_safety = controller_safety(cs, &storage_safety);
                struct command U_complex = controller_complex(cs, &storage_complex);

                if (decide(cs, U_complex, 0.2) == 1 && (remaining_safety_cycles <= 0)) {

                    vol_right = U_complex.u1;
                    vol_left = U_complex.u2;
                } else {
                    vol_right = U_safety.u1;
                    vol_left = U_safety.u2;
                    remaining_safety_cycles -= 1;
                }
            }

        //
        //        if (step % 200 == 0) {
        //          if (DEBUG == 1){
        //              printf("restart\n");
        //          }
        //            usleep(RESTART_TIME * 1000000.0);
        //            remaining_safety_cycles = 60;
        //
        //        }

            //      vol_right = 0;
            //      vol_left = 0;
            //      printf ("elev -1/3* pitch : %lf elev + 1.0/3.0*pitch: %lf\n", cs.elevation-0.3333*cs.pitch, cs.elevation + 0.3333*cs.pitch);

            if (vol_right > MAX_VOLTAGE)
                vol_right = MAX_VOLTAGE;
            else if (vol_right < -MAX_VOLTAGE)
                vol_right = -MAX_VOLTAGE;

            if (vol_left > MAX_VOLTAGE)
                vol_left = MAX_VOLTAGE;
            else if (vol_left < -MAX_VOLTAGE)
                vol_left = -MAX_VOLTAGE;


            vol_right = voltage_max_min(vol_right);
            vol_left = voltage_max_min(vol_left);
            
            vTaskDelay(100);
        }
    }    


}

int main(void)
{
    /* Initialize board specified hardware. */
    hardware_init();

    // Get current module clock frequency.
    initConfig.clockRate  = get_uart_clock_freq(BOARD_DEBUG_UART_BASEADDR);

    /* Initialize UART baud rate, bit count, parity, stop bit and direction. */
    UART_Init(BOARD_DEBUG_UART_BASEADDR, &initConfig);

    /* Set UART build-in hardware FIFO Watermark. */
    UART_SetTxFifoWatermark(BOARD_DEBUG_UART_BASEADDR, 16);
    UART_SetRxFifoWatermark(BOARD_DEBUG_UART_BASEADDR, 1);

    /* Finally, enable the UART module */
    UART_Enable(BOARD_DEBUG_UART_BASEADDR);

    /* Create a the APP main task. */
    xTaskCreate(MainTask, "Main Task", configMINIMAL_STACK_SIZE + 4200,
                NULL, tskIDLE_PRIORITY+1, NULL);

    /* Start FreeRTOS scheduler. */
    vTaskStartScheduler();

    /* should never reach this point. */
    while (true);
}

static void UART_SendDataPolling(UART_Type *base, const uint8_t *txBuff, uint32_t txSize)
{
    while (txSize--)
    {
        while (!UART_GetStatusFlag(base, uartStatusTxComplete));
        UART_Putchar(base, *txBuff++);
    }
}

static void UART_ReceiveDataPolling(UART_Type *base, uint8_t *rxBuff, uint32_t rxSize)
{
    while (rxSize--)
    {
        while (!UART_GetStatusFlag(base, uartStatusRxReady));
        *rxBuff = UART_Getchar(base);
        rxBuff++;

        if (UART_GetStatusFlag(base, uartStatusRxOverrun))
            UART_ClearStatusFlag(base, uartStatusRxOverrun);
    }
}

/*******************************************************************************
 * EOF
 ******************************************************************************/
