/*
 * Timer_Counter_header.h
 * This is a timer-counter header file having the following capabilities : 
   1.)Generating square waves with given duty cycle and time period using any three modes : normal , ctc , fast-pwm
   2.)Reading the frequency of input pulse applied at a given pulse 
 * Created: 10-05-2018 15:22:12
 *  Author: Bhushan 
 copyright Bhushan 
 */ 
/* Implementation done using timer_1*/
//generate the square wave with given duty cycle and time period using timer_1 and CTC mode or Fast PWM mode 
//the top value is adjusted using the OCR1A register in CTC mode
//
#ifndef Timer_Counter_header_h
#define Timer_Counter_header_h
#define F_CPU 1000000
#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>
unsigned int time_period;       //time period upto 65536 milliseconds 
unsigned char duty_cycle;       //duty cycle : 0_100_percent 
unsigned char port;
unsigned char pin;
unsigned int generate_square_normal(unsigned int time_period,unsigned char duty_cycle,unsigned char port,unsigned char pin); //for normal mode 50% duty cycle is preferable 
unsigned int* generate_square_fast_pwm(unsigned int time_period,unsigned char duty_cycle,unsigned char port,unsigned char pin); //generate on any pin using the overflow flag  
unsigned int* generate_square_ctc(unsigned int time_period,unsigned int duty_cycle); //directly generate on the OC1A pin 
void read_frequency();              //reads input frequency connected to the ICP1 pin of the uc using Timer1 
#endif 
 