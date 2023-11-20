#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "tm4c123gh6pm.h"
#include "mpu6050.h"
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
char buffer[20];
int sum, scaled_delay, count = 0;
float AXsum = 0, AYsum = 0, AZsum = 0, GXsum = 0, GYsum = 0, GZsum = 0, Tsum = 0;
float AXav = 0, AYav = 0, AZav = 0, GXav = 0, GYav = 0, GZav = 0, Tav = 0;
int main(void)
{
    I2C3_Config();
    //Delay(1);
    GPIO_PORTF_setup();
    MPU_Init();
    UART0_Init();
    int a_x, a_y, a_z, temperature, g_x, g_y, g_z;
    float AX, AY, AZ, T, GX, GY, GZ;
    //float ax_prev, gx_prev;
    char readings[14];
    print("\n Hello World! Gyroscope Here \n");
    while(1){
        //MPU_Init();
        /*
        GPIO_PORTF_DATA_R &= 0xFFF0;
        GPIO_PORTF_DATA_R |= GREEN_LED;
        Delay(50);
        GPIO_PORTF_DATA_R &= ~(GREEN_LED);
        Delay(10);
        */
        ReadBytes(0x68,ACCEL_XOUT_H, 14, readings);
        a_x = (int)((readings[0] << 8)| readings[1]);
        a_y = (int)((readings[2] << 8)| readings[3]);
        a_z = (int)((readings[4] << 8)| readings[5]);

        temperature = (int)((readings[6] << 8)| readings[7]);

        g_x = (int)((readings[8] << 8)| readings[9]);
        g_y = (int)((readings[10] << 8)| readings[11]);
        g_z = (int)((readings[12] << 8)| readings[13]);

        //AX = (float)a_x/16384.0;
        /*
        AX = (float)a_x/2048.0;
        AY = (float)a_y/2048.0;
        AZ = (float)a_z/2048.0;*/
        AX = (float)a_x/16384.0;
        AY = (float)a_y/16384.0;
        AZ = (float)a_z/16384.0;
        T = ((float)temperature/340.00)+36.53;
        GX = (float)g_x/131.0;
        GY = (float)g_y/131.0;
        GZ = (float)g_z/131.0;

        //ax_prev = AX;
        //gx_prev = GX;
        sum = AX + AY + AZ + GX + GY + GZ;

        //if (T < 40 && sum > 250){
        if (T > 0){
            //sum = AX + AY + AZ + GX + GY + GZ;
            count++;
            AXsum += AX;
            AYsum += AY;
            AZsum += AZ;
            //GX = GX - 312.99;
            GXsum += GX;
            GYsum += GY;
            GZsum += GZ;
            T = T -10;
            Tsum += T;
            /*
            GPIO_PORTF_DATA_R = BLUE_LED;
            scaled_delay = (sum/250)*10;
            Delay(scaled_delay);
            //Delay(10);
            //GPIO_PORTF_DATA_R = 0;
            GPIO_PORTF_DATA_R &= ~(BLUE_LED);
            */
        }
        if (count == 200) { //100 is good
            // Calculate average
            //float average = (float)sum / count;
            AXav = AXsum / count;
            AYav = AYsum / count;
            AZav = AZsum / count;
            GXav = GXsum / count;
            GYav = GYsum / count;
            GZav = GZsum / count;
            Tav = Tsum / count;

            sprintf(buffer, "Ax = %.2f \t", AXav);
            print(buffer);

            sprintf(buffer, "Ay = %.2f \t", AYav);
            print(buffer);
            sprintf(buffer, "Az = %.2f \t\n", AZav);
            print(buffer);

            sprintf(buffer, "Temperature = %.2f \t", Tav);
            print(buffer);

            sprintf(buffer, "Gx = %.2f \t", GXav);
            print(buffer);
            sprintf(buffer, "Gy = %.2f \t", GYav);
            print(buffer);
            sprintf(buffer, "Gz = %.2f \t\n", GZav);
            print(buffer);

            if ((AXav + AYav + AZav) > 1){
                GPIO_PORTF_DATA_R = GREEN_LED;
                Delay(75);
                GPIO_PORTF_DATA_R = 0;
            }
            if ((GXav + GYav + GZav) > 75){
                GPIO_PORTF_DATA_R = BLUE_LED;
                Delay(75);
                GPIO_PORTF_DATA_R = 0;
            }

            // Reset variables for the next set of readings
            AXsum = 0, AYsum = 0, AZsum = 0, GXsum = 0, GYsum = 0, GZsum = 0, Tsum = 0;
            AXav = 0, AYav = 0, AZav = 0, GXav = 0, GYav = 0, GZav = 0, Tav = 0;
            //sum = 0;
            count = 0;

        }


        //Delay(10);

    }
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
    WriteByte(0x68,PWR_MGMT_1,  0x01);//use pll with x axis gyro reference, no sleep or cycle
    WriteByte(0x68,SMPLRT_DIV, 0x07);
    WriteByte(0x68,CONFIG, 0x00);//dlpf_cfg=7, dlpf is off, no ext sync, pg13
    WriteByte(0x68,ACCEL_CONFIG,0x00);//2g linear acceleration range, try 0x18 for full acceleration range
    //WriteByte(0x68,ACCEL_CONFIG,0x18);//16g full acceleration range
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
