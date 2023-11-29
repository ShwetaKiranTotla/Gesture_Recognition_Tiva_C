#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "tm4c123gh6pm.h"
#include "mpu6050.h"
#include "main.h"

void GPIO_PORTF_setup(void){
    SYSCTL_RCGC2_R |= 0x00000020;      /* enable clock to GPIOF */
    GPIO_PORTF_LOCK_R = 0x4C4F434B;     /* unlock commit register */
    GPIO_PORTF_CR_R = 0x01;             /* make PORTF0 configurable */
    GPIO_PORTF_DIR_R = 0x0E;            /* set PORTF3+PORTF2+PORTF1 pin as output (LED) pin */
                                        /* and PORTF4 and PORTF0 as input, SW1 is on PORTF4  and SW2 is PORTF0*/
    GPIO_PORTF_DEN_R = 0x1F;            /* set PORTF pins 4-3-2-1 as digital pins */
    GPIO_PORTF_PUR_R = 0x11;            /* enable pull up for pin 4 and 0 */
    GPIO_PORTF_LOCK_R = 0;     /* lock commit register */
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

char message[20];
int sum = 0, count = 0;
float average;
int main(void)
{
    I2C3_Config();
    GPIO_PORTF_setup();
    MPU_Init();
    UART0_Init();
    int a_x;
    float AX;
    char readings[2];
    int interrupt_status[1];
    int int_val;
    while(1){
        //Check if Data Ready Interrupt is Generated
        ReadByte(0x68,INT_STATUS, 1, interrupt_status);
        int_val = interrupt_status[0];
        //If yes, read sensor data
        if(int_val & 0x1){
            ReadBytes(0x68,ACCEL_XOUT_H, 2, readings);
            a_x = (int)((readings[0] << 8)| readings[1]);
            sprintf(message, "Ax = %d \t", a_x <<14);
            print(message);
            sum += a_x;
            count++;
            // Check if 8 values have been read
            if (count == 8) {
                // Calculate average
                average = sum << 17;

                // Reset variables for the next set of readings
                sum = 0;
                count = 0;

            }
            if (average > 1){
                GPIO_PORTF_DATA_R = BLUE_LED;
                Delay(50);
                GPIO_PORTF_DATA_R = 0;
            }
        }
    }
}
