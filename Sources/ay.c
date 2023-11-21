#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "tm4c123gh6pm.h"
#include "mpu6050.h"
#define MPU_ADDR 0x68
//page 1024 datasheet
/*
 * Changes made
 * 1.Changed return of write bytes from char to int
 * 2. Changed the order of write bytes, first power management instead of sample rate
 * 3. Doing read modify write for te mcs register instead of just write
 */
//GPIO PORTF LEDs
#define RED_LED (1 << 1)
#define BLUE_LED (1 << 2)
#define GREEN_LED (1 << 3)
/*
 * Function Prototypes
 */
void I2C3_Config(void); //Configure gpio port D for I2C3, Set I2C3 as master and set data tx speed
/* Data Transmission speed of 100kbps (standard mode)
(1 + TIME_PERIOD ) = SYS_CLK /(2*( SCL_LP + SCL_HP ) * I2C_CLK_Freq )
TIME_PERIOD = 16 ,000 ,000/(2(6+4) *100000) - 1 = 7
*/
void Delay(unsigned long counter);//In delay.c
void MPU_Init(void); //Initialize the MPU
static int I2C3_wait_till_busy(void); //wait till i2c master is busy
int WriteByte(int SlaveAddr, int MemAddr, int data); //Write data to the MemAddr of the SlaveAddr
void UART0_Init(void); //Initialize uart0 for reading the values
void UART0_SendChar(char data);
void print(char* string); //prints string by sending values to uart0 which can be read on terminal
int ReadBytes(int SlaveAddr, int MemAddr, int byte_count, char* data);
int ReadByte(int SlaveAddr, int MemAddr, int byte_count, int* data);
/*
 * variable definitions
 */
int success;
void GPIO_PORTF_setup(void){
    SYSCTL_RCGC2_R |= 0x00000020;      /* enable clock to GPIOF */
    GPIO_PORTF_LOCK_R = 0x4C4F434B;     /* unlock commit register */
    GPIO_PORTF_CR_R = 0x01;             /* make PORTF0 configurable */
    GPIO_PORTF_DIR_R = 0x0E;            /* set PORTF3+PORTF2+PORTF1 pin as output (LED) pin */
                                        /* and PORTF4 and PORTF0 as input, SW1 is on PORTF4  and SW2 is PORTF0*/
    GPIO_PORTF_DEN_R = 0x1F;            /* set PORTF pins 4-3-2-1 as digital pins */
    GPIO_PORTF_PUR_R = 0x11;            /* enable pull up for pin 4 and 0 */
    //GPIO_PORTF_LOCK_R = 0;     /* lock commit register */
}
char message[20];
int sum = 0, count = 0;
float average;
int main(void)
{
    I2C3_Config();
    //Delay(1);
    GPIO_PORTF_setup();
    MPU_Init();
    UART0_Init();
    int a_x;
    float AX;
    char readings[4];
    int interrupt_status[1];
    int int_val;
    while(1){

        ReadByte(0x68,INT_STATUS, 1, interrupt_status);
        int_val = interrupt_status[0];
        /*
        if(int_val & 0x1){
            ReadBytes(0x68,ACCEL_XOUT_H, 2, readings);
            a_x = (int)((readings[0] << 8)| readings[1]);
            AX = (float)a_x/16384.0;
            sprintf(message, "Ax = %.2f \t", AX);
            print(message);
            sum += AX;
            count++;
            // Check if 200 values have been read
            if (count == 20) {
                // Calculate average
                average = (float)sum / count;

                // Reset variables for the next set of readings
                sum = 0;
                count = 0;

            }
            */
        if(int_val & 0x1){
            ReadBytes(0x68,ACCEL_YOUT_H, 2, readings);
            a_x = (int)((readings[0] << 8)| readings[1]);
            AX = (float)a_x/16384.0;
            sprintf(message, "Ay = %.2f \t", AX);
            print(message);
            sum += AX;
            count++;
            // Check if 200 values have been read
            if (count == 8) {
                // Calculate average
                average = (float)sum / count;

                // Reset variables for the next set of readings
                sum = 0;
                count = 0;

            }
            if (average > 2.2){
            //if (AX > 3.8 && AX < 4){
                GPIO_PORTF_DATA_R = GREEN_LED;
                Delay(50);
                GPIO_PORTF_DATA_R = 0;
            }
            //Delay(50);
        }
        //Delay(10);
        /*
        ReadBytes(0x68,ACCEL_XOUT_H, 2, readings);
        a_x = (int)((readings[0] << 8)| readings[1]);
        AX = (float)a_x/16384.0;
        sprintf(message, "Ax = %.2f \t", AX);
        print(message);
        Delay(50);
        //MPU_Init();
*/
    }
}
int ReadByte(int SlaveAddr, int MemAddr, int byte_count, int* data){
    int read_error;
    if (byte_count <= 0)
        return -1;
    I2C3_MSA_R |= (MPU_ADDR) << 1; //tx mode to send the register address
   // I2C3_MSA_R &= ~(0x1); //tx operation
    I2C3_MDR_R |= MemAddr;
    I2C3_MCS_R &= ~(0xF);
    I2C3_MCS_R |= 0x3; //Repeated START condition followed by a TRANSMIT (master remains in Master Transmit state).
    read_error = I2C3_wait_till_busy();       // wait till write completes
    if (read_error)
        return read_error;
    I2C3_MSA_R |= (MPU_ADDR << 1) + 1 ;//rx mode to read values
    if (byte_count == 1){
        I2C3_MCS_R &= ~(0xF);
        I2C3_MCS_R |= 0x7; //Repeated START condition followed by RECEIVE and STOP condition(master goes to Idle state)
    }
    else{
        I2C3_MCS_R &= ~(0xF);
        I2C3_MCS_R |= 0xB; //Repeated START condition followed by RECEIVE (master remains in Master Receive state).
    }
    read_error = I2C3_wait_till_busy();       // wait till write completes
    if (read_error)
        return read_error;
    *data++ = I2C3_MDR_R;
    if (--byte_count == 0){//single byte read
        while(I2C3_MCS_R & 0x40); //wait for bus to become idle
        return 0;       // no error
    }

    I2C3_MCS_R &= ~(0xF);
    I2C3_MCS_R |= 0x5; //RECEIVE followed by STOP condition (master goes to Idle state)
    read_error = I2C3_wait_till_busy();   // wait till write completes
    *data = I2C3_MDR_R; //save data read
    while(I2C3_MCS_R & 0x40); //wait for bus to become idle

    return 0; //no error

}
int ReadBytes(int SlaveAddr, int MemAddr, int byte_count, char* data){
    int read_error;
    if (byte_count <= 0)
        return -1;
    I2C3_MSA_R |= (MPU_ADDR) << 1; //tx mode to send the register address
   // I2C3_MSA_R &= ~(0x1); //tx operation
    I2C3_MDR_R |= MemAddr;
    I2C3_MCS_R &= ~(0xF);
    I2C3_MCS_R |= 0x3; //Repeated START condition followed by a TRANSMIT (master remains in Master Transmit state).
    read_error = I2C3_wait_till_busy();       // wait till write completes
    if (read_error)
        return read_error;
    I2C3_MSA_R |= (MPU_ADDR << 1) + 1 ;//rx mode to read values
    if (byte_count == 1){
        I2C3_MCS_R &= ~(0xF);
        I2C3_MCS_R |= 0x7; //Repeated START condition followed by RECEIVE and STOP condition(master goes to Idle state)
    }
    else{
        I2C3_MCS_R &= ~(0xF);
        I2C3_MCS_R |= 0xB; //Repeated START condition followed by RECEIVE (master remains in Master Receive state).
    }
    read_error = I2C3_wait_till_busy();       // wait till write completes
    if (read_error)
        return read_error;
    *data++ = I2C3_MDR_R;
    if (--byte_count == 0){//single byte read
        while(I2C3_MCS_R & 0x40); //wait for bus to become idle
        return 0;       // no error
    }
    while (byte_count > 1){
        I2C3_MCS_R &= ~(0xF);
        I2C3_MCS_R |= 0x9; //RECEIVE operation (master remains in Master Receive state).
        read_error = I2C3_wait_till_busy();       // wait till write completes
        if (read_error)
            return read_error;
        byte_count--;
        *data++ = I2C3_MDR_R; //save data read
    }

    I2C3_MCS_R &= ~(0xF);
    I2C3_MCS_R |= 0x5; //RECEIVE followed by STOP condition (master goes to Idle state)
    read_error = I2C3_wait_till_busy();   // wait till write completes
    *data = I2C3_MDR_R; //save data read
    while(I2C3_MCS_R & 0x40); //wait for bus to become idle

    return 0; //no error

}
void MPU_Init(void){
    // step wise checking
    /*
    success = WriteByte(0x68,PWR_MGMT_1,  0x01);//use pll with x axis gyro reference, no sleep or cycle
    success = WriteByte(0x68,SMPLRT_DIV, 0x07);
    success = WriteByte(0x68,CONFIG, 0x00);//dlpf_cfg=7, dlpf is off, no ext sync, pg13
    success = WriteByte(0x68,ACCEL_CONFIG,0x00);//2g linear acceleration range, try 0x18 for full acceleration range
    success = WriteByte(0x68,GYRO_CONFIG,0x18);//+-2000deg/s gyro range
    success = WriteByte(0x68,INT_ENABLE, 0x01);//enable data ready interrupt

    if (success==0){
        GPIO_PORTF_DATA_R &= 0xFFF0;
        GPIO_PORTF_DATA_R |= GREEN_LED;
        Delay(50);
        GPIO_PORTF_DATA_R &= ~(GREEN_LED);
        Delay(50);
    }
*/
    WriteByte(0x68,PWR_MGMT_1,  0x01);//use pll with x axis gyro reference, no sleep or cycle
    WriteByte(0x68,SMPLRT_DIV, 0x07);
    WriteByte(0x68,CONFIG, 0x00);//dlpf_cfg=7, dlpf is off, no ext sync, pg13
    WriteByte(0x68,ACCEL_CONFIG,0x00);//2g linear acceleration range, try 0x18 for full acceleration range
    WriteByte(0x68,GYRO_CONFIG,0x18);//+-2000deg/s gyro range
    WriteByte(0x68,INT_ENABLE, 0x01);//enable data ready interrupt
}

static int I2C3_wait_till_busy(void)
{
    while(I2C3_MCS_R & 1);   // wait until I2C master is not busy
    return I2C3_MCS_R & 0xE; // return I2C error
}


WriteByte(int SlaveAddr, int MemAddr, int data){
    int error;
    I2C3_MSA_R |= (MPU_ADDR) << 1;
   // I2C3_MSA_R &= ~(0x1); //tx operation
    I2C3_MDR_R |= MemAddr;
    I2C3_MCS_R &= ~(0xF);
    I2C3_MCS_R |= 0x3; //Repeated START condition followed by a TRANSMIT (master remains in Master Transmit state).
    error = I2C3_wait_till_busy();       // wait till write completes
    if (error)
        return error;
    //data transmit
    I2C3_MDR_R = data;
    I2C3_MCS_R &= ~(0xF);
    I2C3_MCS_R |= 0x5;//TRANSMIT followed by STOP condition (master goes to Idle state)
    while(I2C3_MCS_R & 0x40); //wait for bus to become idle
    error = I2C3_MCS_R & 0xE;
    if (error) return error;
    return 0;

}
void I2C3_Config(void){
    SYSCTL_RCGCGPIO_R |= 0x8; //Enable GPIOD Clock
    SYSCTL_RCGCI2C_R |= 0x8; //Enable i2c3 clock
    GPIO_PORTD_DEN_R = 0x3; //ref datasheet page 683
    // Configure PD0 and PD1 for i2c3
    GPIO_PORTD_AFSEL_R = 0x3;
    GPIO_PORTD_PCTL_R = 0x33;
    //GPIO_PORTD_ODR_R = 0x2;
    GPIO_PORTD_ODR_R = 0x2; //open drain enable
    I2C3_MCR_R |= 0x10; //Configure as master
    I2C3_MTPR_R |= 0x7;
}

void UART0_Init(void) {
    SYSCTL_RCGCUART_R |= 0x01;   // Enable UART0
    SYSCTL_RCGCGPIO_R |= 0x01;   // Enable GPIO Port A
    // Configure UART0 pins (PA0 and PA1)
    GPIO_PORTA_AFSEL_R |= 0x03;  // Enable alternate function for PA0 and PA1

    GPIO_PORTA_PCTL_R |= 0x00000011;  // Configure PA0 and PA1 as UART pins
    GPIO_PORTA_DEN_R |= 0x03;    // Enable digital function on PA0 and PA1
    // Configure UART0 settings (baud rate, data bits, stop bits, etc.)
    UART0_CTL_R &= ~0x01;        // Disable UART0
    UART0_IBRD_R = 104;          // IBRD = int(16,000,000 / (16 * 9600)) = 104
    UART0_FBRD_R = 11;           // FBRD = int(0.1668 * 64 + 0.5) = 11
    UART0_LCRH_R = 0x70;         // 8-bit word length, enable FIFO
    UART0_CTL_R |= 0x301;        // Enable UART0, TX, and RX
}

void UART0_SendChar(char data) {
    while ((UART0_FR_R & 0x20) != 0);  // Wait until TX buffer is not full
    UART0_DR_R = data;                // Send the character
}

void print(char* string) {
    while (*string) {
        UART0_SendChar(*string);
        string++;
    }
}
