## The Project
This repository contains files for gesture recognition (in C) using the TM4C123GH6PM MCU and MPU6050 6 axis IMU using TI CCS.  


This project was done when I had a fairly good grasp on programming the ARM Cortex core and Tiva C series microcontroller. Thanks to Abhijit Sir, I am not dependant on libraries and have directly accessed the registers.

## The Journey: From Idea to Code
As this was the first time with MPU6050, I started with understanding the register from the official documentation of the sensor. Based on this a header file containing register address was compiled.

Simultaneously I was also working on configuring I2C with the microcontroller as the MPU uses I2C to communicate (there are other involved options as well).

There were erratic samples in the data being read (the values of linear and angular acceleration would be quite high even when the device was stationary). Considering the possibility of oversampling, a delay was added after each iteration but that did not help (because the data wasn't being oversampled!). Then, after some brainstorming, to filter noise from the samples, a Parks McCollen filter was simulated in MATLAB, but I did not implement it, instead I implemented a combination of two facts: one, when the values of acceleration is erratic, so is the value of temperature, so only those samples which have a reasonable temperature were read and a moving average filter over these data.
I also used the data ready interrupt of the MPU to read samples and it helped.

## References
1. [Github Library for various applications of MPU6050 with Arduino](https://github.com/jarzebski/Arduino-MPU6050/tree/dev)
2. [Github Library for MPU6050 and ESP8266 based gesture recognition program on Adafruit ARM core using Arduino IDE](https://github.com/cookiestroke/Gesture-Recognition)
3. [Interface MPU6050 Accelerometer and Gyroscope Sensor with Arduino](https://lastminuteengineers.com/mpu6050-accel-gyro-arduino-tutorial/)  
An in depth explaination of the working of the sensor, it's measurements and its interfacing on Arduino.
4. [MPU6050 source C file for Tiva C series](https://github.com/yuvadm/tiva-c/blob/master/sensorlib/mpu6050.c)
5. [The MPU6050 Explained by Programming Robots](https://mjwhite8119.github.io/Robots/mpu6050)  
Arduino code for the MPU6050 along with sensor calibration.
<br> </br>

6. [About the Digital Motion Processor on MPU6050  by stackexchange](https://electronics.stackexchange.com/questions/161291/mpu6050-dmp-values-read#:~:text=The%20code%20executed%20by%20the,3kB%20array%20of%20raw%20bytes.)
7. [I2C device Library](https://github.com/jrowberg/i2cdevlib/tree/master)
8. [I2C communication with Tiva C series](https://microcontrollerslab.com/i2c-communication-tm4c123g-tiva-c-launchpad/)
9. [Another Github Library of MPU6050 with Arduino](https://github.com/ElectronicCats/mpu6050/tree/master)
10. [Arduino and MPU6050 on Github with Error Calculation](https://howtomechatronics.com/tutorials/arduino/arduino-and-mpu6050-accelerometer-and-gyroscope-tutorial/)
11. [MPU6050 Tiva C Launchpad](https://microcontrollerslab.com/mpu6050-gyroscope-accelerometer-sensor-interfacing-with-tm4c123g-tiva-c-launchpad/)
12. [Mpu6050: Utilizing Tiva C Direct Register Mode for Power Management of MPU6050](https://copyprogramming.com/howto/mpu6050-power-management-using-tiva-c-direct-register-mode)
13. [Interfacing MPU6050 with STM32](https://controllerstech.com/how-to-interface-mpu6050-gy-521-with-stm32/)
14. [Electronics Stack Exchange topic on inconsistent MPU6050 values](https://electronics.stackexchange.com/questions/364943/mpu6050-showing-inconsistent-angle-values)
 
