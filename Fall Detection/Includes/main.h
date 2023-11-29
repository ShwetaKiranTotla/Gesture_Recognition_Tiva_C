/*
 * main.h
 *
 *  Created on: 17-Nov-2023
 *      Author: Shweta Kiran
 */

#ifndef MAIN_H_
#define MAIN_H_

//GPIO PORTF LEDs
#define RED_LED (1 << 1)
#define BLUE_LED (1 << 2)
#define GREEN_LED (1 << 3)
/*
 * Function Prototypes
 */
void GPIO_PORTF_setup(void); //GPIO setup for LED Control
void I2C3_Config(void); //Configure gpio port D for I2C3, Set I2C3 as master and set data tx speed
/* Data Transmission speed of 100kbps (standard mode)
(1 + TIME_PERIOD ) = SYS_CLK /(2*( SCL_LP + SCL_HP ) * I2C_CLK_Freq )
TIME_PERIOD = 16 ,000 ,000/(2(6+4) *100000) - 1 = 7
*/
void Delay(unsigned long counter);//In delay.c
void UART0_Init(void); //Initialize uart0 for reading the values
void UART0_SendChar(char data);
void print(char* string); //prints string by sending values to uart0 which can be read on terminal

/*
 * variable definitions
 */
int success;


#endif /* MAIN_H_ */
