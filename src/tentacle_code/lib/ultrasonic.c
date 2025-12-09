#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "ultrasonic.h"
#include <util/delay.h>
#include <math.h>  


#define TRIG_PIN  PD2
#define ECHO_PIN  PD3

#define TRIG_DDR  DDRD
#define TRIG_PORT PORTD

#define ECHO_DDR  DDRD
#define ECHO_PINR PIND
#define MIN_ECHO_TICKS  300     
#define MAX_ECHO_TICKS  50000  


volatile uint16_t echo_start_ticks = 0;
volatile uint16_t echo_end_ticks   = 0;
volatile uint8_t  measuring        = 0;
volatile uint8_t  echo_captured    = 0;

volatile float current_distance_cm = 0.0f;
static float last_valid_distance = 0.0f;

extern uint8_t angry_mode;



//static float dist_lpf = 0;     

ISR(INT1_vect)
{
    uint8_t echo_high = PIND & (1 << ECHO_PIN);

    if (echo_high) {
        // ===== Rising edge: echo goes HIGH, start timing =====
        echo_start_ticks = TCNT3;
        measuring = 1;

        // Switch to falling edge: ISC11=1, ISC10=0
        EICRA |=  (1 << ISC11);
        EICRA &= ~(1 << ISC10);
    } 
    else {
        // ===== Falling edge: echo goes LOW, end timing =====
        if (!measuring) {
            
            EICRA |= (1 << ISC11) | (1 << ISC10);  
            return;
        }

        echo_end_ticks = TCNT3;
        measuring = 0;

        uint16_t start = echo_start_ticks;
        uint16_t end   = echo_end_ticks;
        uint16_t delta;

        if (end >= start) {
            delta = end - start;
        } else {
            delta = (uint16_t)(end + 65536u - start);
        }

       
        if (delta < MIN_ECHO_TICKS || delta > MAX_ECHO_TICKS) {
          
            EICRA |= (1 << ISC11) | (1 << ISC10); 
            return;
        }

        // tick → us
        float pulse_us = delta * 0.5f;
        float d = (pulse_us * 0.0343f) / 2.0f;  // cm

    
        if (last_valid_distance > 0.1f) {
            float jump = d - last_valid_distance;
           
            if (fabsf(jump) > 40.0f) {
             
                EICRA |= (1 << ISC11) | (1 << ISC10);
                return;
            }

           
            current_distance_cm = 0.6f * last_valid_distance + 0.4f * d;
        } else {
            current_distance_cm = d;
        }

        last_valid_distance = current_distance_cm;
        echo_captured = 1;

       
        EICRA |= (1 << ISC11) | (1 << ISC10);
    }
}


// =========================================================
//  Burst measurement: 10us pulse
// =========================================================
static void ultrasonic_start_measurement(void)
{
    if (measuring) return;

    echo_captured = 0;
    measuring = 1;

    TRIG_PORT |= (1 << TRIG_PIN);
    _delay_us(10);
    TRIG_PORT &= ~(1 << TRIG_PIN);
}


// =========================================================
//  Initialization
// =========================================================
void ultrasonic_init(void)
{
    // TRIG output
    TRIG_DDR |=  (1 << TRIG_PIN);
    TRIG_PORT &= ~(1 << TRIG_PIN);

    // ECHO input
    ECHO_DDR &= ~(1 << ECHO_PIN);

    // Timer3: prescaler=8 (2MHz → 0.5us/tick)
    TCCR3A = 0;
    TCCR3B = 0;
    TCCR3B |= (1 << CS31);
    TCNT3 = 0;

    // INT1 Rising edge
    EICRA |= (1<<ISC11) | (1<<ISC10);
    EIMSK |= (1<<INT1);

    current_distance_cm = 0;
    //dist_lpf = 0;

    ultrasonic_start_measurement();
}


// =========================================================
//  Distance API
// =========================================================
float ultrasonic_get_distance(void)
{
    return current_distance_cm;
}

SpeedLevel ultrasonic_get_speed_nonblocking(void)
{
    
    const float dt = 0.30f;  // 300ms

    static float  last_distance = 0.0f;
    static uint8_t first = 1;



    if (!measuring && echo_captured) {
        echo_captured = 0;  
        float d = current_distance_cm;

        if (first) {
            last_distance = d;
            first = 0;
            ultrasonic_start_measurement();
            return SPEED_NONE;
        }

        float delta_d = d - last_distance;

    
        if (fabsf(delta_d) > 20.0f) {
          
            ultrasonic_start_measurement();
            return SPEED_NONE;
        }

        float speed = delta_d / dt;  // cm/s

      
        if (fabsf(speed) < 5.0f) {
            speed = 0.0f;
        }

        last_distance = d;

      

        
        _delay_ms(300); 
        ultrasonic_start_measurement();

       
        if (speed < -5.0f && speed > -15.0f) {
            if (angry_mode==1) {
                return SPEED_NONE;  
            }
            return SPEED_SLOW;
        } else if (speed <= -15.0f) {
            return SPEED_FAST;
        } else if (speed > 4.0f) {
            return SPEED_BACK;
        } else if (speed == 0.0f) {
            return SPEED_NONE;
        } else {
            return SPEED_NONE;
        }
    }

 
    if (!measuring) {
        ultrasonic_start_measurement();
    }

    return SPEED_NONE;
}
