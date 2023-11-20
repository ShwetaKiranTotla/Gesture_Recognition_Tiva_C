#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "tm4c123gh6pm.h"


/*
 * uart.c
 *
 *  Created on: 20-Nov-2023
 *      Author: Shweta KIran
 */
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



