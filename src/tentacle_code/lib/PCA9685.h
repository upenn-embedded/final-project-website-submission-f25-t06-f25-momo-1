
#ifndef PCA9685_H_
#define PCA9685_H_

#define F_CPU 16000000UL 

//#include <xc.h>
#include <stdint.h>
#include <math.h>
#include <util/delay.h> 


#define PCA9685_ADDR    0x40 
#define MODE1           0x00
#define PRESCALE        0xFE
#define LED0_ON_L       0x06

#define SERVOMIN  102  
#define SERVOMAX  512


void I2C_Init(void) ;
void I2C_Start(void) ;
void I2C_Stop(void) ;
void I2C_Write(uint8_t data) ;


void PCA9685_WriteByte(uint8_t reg, uint8_t value);
void PCA9685_Init(float freq);
void PCA9685_SetPWM(uint8_t channel, uint16_t on, uint16_t off);
void SetServoAngle(uint8_t channel, int angle);

#endif /* PCA9685_H_ */