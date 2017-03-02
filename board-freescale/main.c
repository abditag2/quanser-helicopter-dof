

#include "FreeRTOS.h"
#include "task.h"
#include "board.h"
#include "debug_console_imx.h"
#include "gpio_ctrl.h"
#include "hw_timer.h"
#include "semphr.h"
#include "string.h"



#include "gpio_pins.h"
#include "gpio_imx.h"
#include "uart_imx.h"
#include "wdog_imx.h"


static uint32_t wdReset = 20;
static void UART_SendDataPolling(UART_Type *base, const uint8_t *txBuff, uint32_t txSize);
static void UART_ReceiveDataPolling(UART_Type *base, uint8_t *rxBuff, uint32_t rxSize);

////////////////////////////////////////////////////////////////////////////////
// Static Variable
////////////////////////////////////////////////////////////////////////////////

//static SemaphoreHandle_t xSemaphore;


////////////////////////////////////////////////////////////////////////////////
// Quanser Code Starts Here
////////////////////////////////////////////////////////////////////////////////

#define DEBUG           0
#define RECORD          0
#define HX_SIZE         52

#define SIM_STEP        0.02
#define PERIOD          0.05
#define RESTART_TIME    0.

#define MAX_VOLTAGE     3

//unsigned char readData[2]= {0xCC,0xCC};
// /unsigned char writeData[2]={0xDD,0xBB};


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
{-0.937749,-0.347314,0,0.000000,0.000000,0},
{1.000000,0.000000,0,0.000000,0.000000,0},
{0.000000,0.000000,0,1.000000,0.000000,0},
{0.000000,0.000000,0,0.000000,1.000000,0},
{-0.937749,0.347314,0,0.000000,0.000000,0},
{-1.000000,0.000000,0,0.000000,0.000000,0},
{0.000000,0.000000,0,-1.000000,0.000000,0},
{0.000000,0.000000,0,0.000000,-1.000000,0},
{-0.920831,0.341049,0,-0.184166,0.042875,0},
{0.980581,0.000000,0,0.196116,0.000000,0},
{0.976164,0.092968,0,0.195233,0.018594,0},
{0.976164,-0.092968,0,0.195233,-0.018594,0},
{0.982846,0.026635,0,0.182186,0.010654,0},
{0.983313,0.033791,0,0.178416,0.011006,0},
{-0.980581,0.000000,0,-0.196116,0.000000,0},
{-0.942434,-0.177818,0,-0.280952,-0.035564,0},
{-0.920831,-0.341049,0,-0.184166,-0.042875,0},
{-0.939588,0.229568,0,-0.251069,0.037920,0},
{-0.929818,0.247341,0,-0.269327,0.041529,0},
{-0.945071,0.207015,0,-0.250444,0.035553,0},
{0.730320,-0.429600,0,0.524112,-0.085920,0},
{0.730320,0.429600,0,0.524112,0.085920,0},
{0.983313,-0.033791,0,0.178416,-0.011006,0},
{0.982846,-0.026635,0,0.182186,-0.010654,0},
{-0.919538,-0.340570,0,-0.183908,-0.068114,0},
{-0.929818,-0.247341,0,-0.269327,-0.041529,0},
{-0.945071,-0.207015,0,-0.250444,-0.035553,0},
{-0.924294,0.088028,0,-0.369718,0.035211,0},
{-0.942434,0.177818,0,-0.280952,0.035564,0},
{-0.976164,0.092968,0,-0.195233,0.018594,0},
{-0.942675,0.223627,0,-0.244964,0.036715,0},
{-0.919538,0.340570,0,-0.183908,0.068114,0},
{-0.942675,-0.223627,0,-0.244964,-0.036715,0},
{-0.939588,-0.229568,0,-0.251069,-0.037920,0},
{-0.976164,-0.092968,0,-0.195233,-0.018594,0},
{-0.924294,-0.088028,0,-0.369718,-0.035211,0},
{0.924294,0.088028,0,0.369718,0.035211,0},
{0.924294,-0.088028,0,0.369718,-0.035211,0},


};

double hx[HX_SIZE][1] = {
{0.114896},
{0.119671},
{0.199735},
{1.300000},
{0.114896},
{0.119671},
{0.199735},
{1.300000},
{0.164176},
{0.136959},
{0.197136},
{0.197136},
{0.157410},
{0.160876},
{0.136959},
{0.160740},
{0.164176},
{0.156719},
{0.165424},
{0.152473},
{0.496346},
{0.496346},
{0.160876},
{0.157410},
{0.184185},
{0.165424},
{0.152473},
{0.185233},
{0.160740},
{0.137058},
{0.153904},
{0.184185},
{0.153904},
{0.156719},
{0.137058},
{0.185233},
{0.242119},
{0.242119},
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
struct state sps;

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


struct command controller_safety(struct state sp, struct state x, struct controller_storage* cs){

    struct command U;

    cs->int_travel +=  x.travel;
    cs->int_pitch +=  x.pitch;
    cs->int_elevation +=  x.elevation;

    U.u1 = -6.5 * (x.elevation-sp.elevation) - .701 * x.pitch  - 45.7161 * PERIOD * x.d_elevation -3.051 * PERIOD * x.d_pitch ; //-0.0333*cs->int_elevation -0.001*cs->int_pitch;
    U.u2 = -6.5 * (x.elevation-sp.elevation) + .5701 * x.pitch - 45.7529 * PERIOD * x.d_elevation +5.970 * PERIOD*  x.d_pitch; //-0.03*cs->int_elevation +0.001*cs->int_pitch;

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

struct command controller_complex(struct state sp, struct state x, struct controller_storage* cs){
    struct command U;
    //printf("\n:info int_elevation: %lf elevation_conv: %lf int_pitch:%lf, pitch_conv:%lf\n", int_elevation, elevation_conv, int_pitch, pitch_conv);

    cs->int_travel +=  x.travel;
    cs->int_pitch +=  x.pitch;
    cs->int_elevation +=  x.elevation;

    double trav = 30.0 * (x.travel - sp.travel)/100.0;
    double d_trav = PERIOD*45*(x.d_travel )/10.0; 
    //printf("controller d %lf\n",  trav) ; //30*(x.travel-spc.travel)/100.0 ) ;    
    //printf("controller   %lf\n",  d_trav) ; //2*(x.d_travel)/10.0 ) ; 

    //This one is working!
    //right voltage
//        U.u1 =    -6.5 * (x.elevation - sp.elevation)  - .701 * x.pitch + trav  - 45.7161 * PERIOD * x.d_elevation -3.051 * PERIOD * x.d_pitch +d_trav; //-0.0333*cs->int_elevation -0.001*cs->int_pitch;
        //left voltage
//  U.u2  =   -6.5 * (x.elevation - sp.elevation)  + .5701 * x.pitch -trav - 45.7529 * PERIOD * x.d_elevation +5.970 * PERIOD*  x.d_pitch -d_trav; //-0.03*cs->int_elevation +0.001*cs->int_pitch;

    U.u1 =    -6.5 * (x.elevation - sp.elevation)  - .9701 * x.pitch + trav  - 55.7161 * PERIOD * x.d_elevation -7.051 * PERIOD * x.d_pitch +d_trav; //-0.0333*cs->int_elevation -0.001*cs->int_pitch;
        //left voltage
    U.u2  =   -6.5 * (x.elevation - sp.elevation)  + .97701 * x.pitch -trav - 55.7529 * PERIOD * x.d_elevation +10.970 * PERIOD*  x.d_pitch -d_trav; //-0.03*cs->int_elevation +0.001*cs->int_pitch;

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
        struct command U = controller_safety(sps, state_x, &cs);
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
    if (x2.safe == 0)
        return 0;

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


    //while (1){

        


        //if(rxChar == 0xAA){
        rxChar = 0xAA;

        UART_SendDataPolling(BOARD_DEBUG_UART_BASEADDR, &rxChar, 1);
        UART_ReceiveDataPolling(BOARD_DEBUG_UART_BASEADDR, rxChar1, 13);

        sensor_readings[0] = *(unsigned int *)&rxChar1[0];
        sensor_readings[1] = *(unsigned int *)&rxChar1[4];
        sensor_readings[2] = *(unsigned int *)&rxChar1[8];

        tmpSend[0] = (double)sensor_readings[0];
        tmpSend[1] = (double)sensor_readings[1];


            //write_to_serial(tmpSend);

        //break;
        //}
   //}

    return;
}



void MainTask(void *pvParameters)
{
    
        /* .. Writing the control values to the left and write motor......*/
    //printf("Main task is called \r\n");


    int step = 0;
    double vol_step = 0.1;
    int byteCount;
    
    //const TickType_t xDelay = 20 / portTICK_PERIOD_MS;
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

    TickType_t xLastWakeTime;
    const TickType_t xFrequency = 50;

     // Initialise the xLastWakeTime variable with the current time.
     xLastWakeTime = xTaskGetTickCount();
    



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

    //byteCount = sizeof(bufferData1);
    //UART_SendDataPolling(BOARD_DEBUG_UART_BASEADDR, bufferData1, byteCount);
   // printf("Main Task has been triggerred..... \r\n");

    //vTaskDelay(100);
    // send read request
    
    //UART_SendDataPolling(BOARD_DEBUG_UART_BASEADDR, readData, 1);
    //read_from_serial(sensor_readings);

    while(1){


    struct state spc; //set point for safety and complex controllers

        for (step = 0; step < 15000; step++) {

        //        unsigned short int tmparray[4];
        //        tmparray[0] = Q8_dacVTO((vol_right), 1, 10);
        //        tmparray[1] = Q8_dacVTO((vol_left), 1, 10);
        //        ioctl(File_Descriptor, Q8_WR_DAC, tmparray);

            
            voltages[0] = vol_left;
            voltages[1] = vol_right;
           
            //sleep(50)
            vTaskDelayUntil( &xLastWakeTime, xFrequency ); 

            write_to_serial(voltages);
             
            //vTaskDelay(xDelay);
            //Hw_Timer_Delay(50);
            /* .. Reading the Encoder values from the helicopter......*/
            //UART_SendDataPolling(BOARD_DEBUG_UART_BASEADDR, readData, 1);
            read_from_serial(sensor_readings);
            //vTaskDelay(50);
            

        //    vTaskDelay(xDelay);
        //        err = ioctl(File_Descriptor, Q8_ENC, sensor_readings);
        //        if (err != 0) {
        //            perror("Epic Fail first enc read\n");
        //            return -1;
        //        }

            int travel = sensor_readings[0];// - base_travel;
            int pitch = sensor_readings[1];// - base_pitch;
            int elevation = -(sensor_readings[2]) - 300;

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


        // if (step < 30 ){
        //     vol_right = 0;
        //     vol_left = 0;
        // }
        // else 
        // {
        /*
            The logic is that after every restart the safety controller is active for a set amount of time. After that if the
            complex controller's command is safe, it can be used again.
        */

          /*      if (step > 1800)
                {
                    spc.travel = 3.1415/2;
                }
                else if( step > 1600){
                    spc.travel = 3.1415/4;
                }
                else if (step > 1400){
                    spc.travel = 0;
                }
                else if (step > 900){
                    spc.travel = 3.14;
                }
                else if (step > 400){
                    spc.elevation = 0.4;
                    spc.travel = 3.14/2;
                }
                else if (step > 300){
                    sps.elevation = 0.4;
                }*/
                // else if (step > 200){
                //  sps.elevation = 0.3;
                // }
                // else if (step > 150){
                //  sps.elevation = 0.2;    
                // }
                // else if (step > 100){
                //  sps.elevation = 0.1;
                // }
                // else if (step >50){
                //  sps.elevation = 0;
                // }
                    spc.travel = 0;
                    sps.elevation = 0.4;
                    spc.elevation = 0.4;
            
                struct command U_safety = controller_safety(sps, cs, &storage_safety);
                struct command U_complex = controller_complex(spc, cs,  &storage_complex);      
                //printf("remaining_cycle %d\n", remaining_safety_cycles);


                if (step < 100){
                        vol_right = U_safety.u1;
                        vol_left = U_safety.u2;
                }
                else if(decide(cs, U_complex, 0.2) == 1 && (remaining_safety_cycles <= 0 )  ) {   
                     //printf("complex controller\n");
                     vol_right = U_complex.u1;
                     vol_left = U_complex.u2;
                }
                else {
                    //printf("safety controller\n");
                    vol_right = U_safety.u1;
                    vol_left = U_safety.u2;
                    remaining_safety_cycles -= 1;
                }
            // }
            
        /*if(step % 200 == 0){

            printf("restart\n");
            //usleep(RESTART_TIME *1000000.0);
            remaining_safety_cycles = 60;

        }*/         




            vol_right = voltage_max_min(vol_right);
            vol_left = voltage_max_min(vol_left);
            
            
           
        }
    }    


}

int main(void)
{

	




    SRC_M4RCR = 0x2a8;

    /* Initialize board specified hardware. */
   // Hw_Timer_Init();

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

    wdog_init_config_t config = {
       .wdw   = false,    /*!< true: suspend in low power wait, false: not suspend */
       .wdt   = true,     /*!< true: assert WDOG_B when timeout, false: not assert WDOG_B */
       .wdbg  = true,     /*!< true: suspend in debug mode, false: not suspend */
       .wdzst = false     /*!< true: suspend in doze and stop mode, false: not suspend */
    };


    WDOG_Init(BOARD_WDOG_BASEADDR, &config);

    /* Enable WDOG interrupt 0.5 second before WDOG timeout */
    NVIC_SetPriority(BOARD_WDOG_IRQ_NUM, 3);
    NVIC_EnableIRQ(BOARD_WDOG_IRQ_NUM);
    /* Refresh WDOG to reload counter */
    WDOG_Refresh(BOARD_WDOG_BASEADDR);
    WDOG_EnableInt(BOARD_WDOG_BASEADDR, 1);


    WDOG_Enable(BOARD_WDOG_BASEADDR, 2);



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
void BOARD_WDOG_HANDLER()
{
	//printf("wdReset %d \n", wdReset)
    WDOG_ClearStatusFlag(BOARD_WDOG_BASEADDR);
    //PRINTF("WDOG was refreshed %d\n\r", wdReset);
    if (wdReset){
        WDOG_Refresh(BOARD_WDOG_BASEADDR);
        
    }
  

    wdReset = wdReset -1;
}
