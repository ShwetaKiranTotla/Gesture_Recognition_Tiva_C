#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "tm4c123gh6pm.h"
#include "mpu6050.h"

/*
 * mpu6050.c
 *
 *  Created on: 19-Nov-2023
 *      Author: Shweta Kiran
 */

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

static int I2C3_wait_till_busy(void)
{
    while(I2C3_MCS_R & 1);   // wait until I2C master is not busy
    return I2C3_MCS_R & 0xE; // return I2C error
}

void MPU_Init(void){
    WriteByte(0x68,PWR_MGMT_1,  0x01);//use pll with x axis gyro reference, no sleep or cycle
    WriteByte(0x68,SMPLRT_DIV, 0x07);
    WriteByte(0x68,CONFIG, 0x00);//dlpf_cfg=7, dlpf is off, no ext sync, pg13
    WriteByte(0x68,ACCEL_CONFIG,0x00);//2g linear acceleration range, 0x18 for full acceleration range
    WriteByte(0x68,GYRO_CONFIG,0x18);//+-2000deg/s gyro range
    WriteByte(0x68,INT_ENABLE, 0x01);//enable data ready interrupt
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



