#include <avr/io.h>
#include <util/delay.h>
#include "tentacle.h"
#include "PCA9685.h"
#include <math.h>
#define PI 3.1415926
#define DEGREE_TO_RAD(x) ((x)*PI/180.0)

// make sure val is within [min_val, max_val]
int clip(int val, int min_val, int max_val) {
    if (val < min_val) return min_val;
    if (val > max_val) return max_val;
    return val;
}

void Tentacle_Move(float direction_angle, float bending_amount){

    if (bending_amount < 0.0) bending_amount = 0.0;
    if (bending_amount > 1.0) bending_amount = 1.0;

    float amplitude = MAX_BEND * bending_amount;
    float rad = DEGREE_TO_RAD(direction_angle);

    int servo0_angle = clip(CENTER_POS + (int)(amplitude * cos(rad)), 0, 170);
    int servo1_angle = clip(CENTER_POS + (int)(amplitude * cos(rad - 2.0 * PI / 3.0)), 0, 170);
    int servo2_angle = clip(CENTER_POS + (int)(amplitude * cos(rad + 2.0 * PI / 3.0)), 0, 170);

    SetServoAngle(0, servo0_angle);
    SetServoAngle(1, servo1_angle); 
    SetServoAngle(2, servo2_angle);

}

// Breathing motion 
void Motion_Breathing(float direction) {
    
    for (int i = 0; i < 360; i+=2) {
      
        float breath = 0.1 + 0.1 * sin(i * 3.14 / 180.0); 
        
        Tentacle_Move(direction, breath);
        _delay_ms(15); 
    }
}


void Motion_Rotate(float bend_val, int speed_delay) {
    for (int phi = 0; phi < 360; phi += 5) {
        Tentacle_Move(phi, bend_val);
        
        for(int k=0; k<speed_delay; k++) _delay_ms(1); 
    }
}

void Motion_ConicalScan(float bend_val, int speed_delay) {
    for (int phi = 0; phi < 360; phi += 5) {
        Tentacle_Move(phi, bend_val);
        
       
        for(int k=0; k<speed_delay; k++) _delay_ms(1); 
    }
}

void Motion_SpiralOut() {
    float bend = 0.0;
    int phi = 0;
    
    while (bend <= 1.0) {
        Tentacle_Move(phi, bend);
        
        phi += 10;        
        if (phi >= 360) phi -= 360;
        
        bend += 0.005;   
        _delay_ms(10);
    }
    
  
    while (bend > 0) {
        bend -= 0.02;     
        Tentacle_Move(phi, bend); 
        _delay_ms(10);
    }
}


void Motion_Strike(float target_angle) {
    Tentacle_Move(target_angle + 180, 0.2); 
    _delay_ms(500);
    
    Tentacle_Move(target_angle, 1.0);
    _delay_ms(300); 
    
   
    for (float b = 1.0; b >= 0; b -= 0.02) {
        Tentacle_Move(target_angle, b);
        _delay_ms(10);
    }
}

