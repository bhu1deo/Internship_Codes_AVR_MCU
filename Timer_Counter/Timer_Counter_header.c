/*
 * Timer_Counter_header.h
 * This is a timer-counter header file having the following capabilities : 
   1.)Generating square waves with given duty cycle and time period using any three modes : normal , ctc , fast-pwm
   2.)Reading the frequency of input pulse applied at a given pulse 
 * Created: 10-05-2018 15:22:12
 *  Author: Bhushan Deo
 *copyright Bhushan Deo , legalities to be dealt with at the Bombay High Court (India)
 */ 
 
#define F_CPU 1000000
#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>
#include<math.h>
#include "Timer_Counter_header.h"
unsigned int temp;
unsigned char porti;
unsigned char pini;
unsigned int* pnt;
unsigned int array[2];
unsigned int array_new[4];
unsigned int generate_square_normal(unsigned int time_period,unsigned char duty_cycle,unsigned char port,unsigned char pin)
{
	sei();
	if(port=='A')
	{
		DDRA|=(1<<pin);
	}
	else if(port=='B')
	{
		DDRB|=(1<<pin);
	}
	else if(port=='C')
	{
		DDRC|=(1<<pin);
	}
	else if(port=='D')
	{
		DDRD|=(1<<pin);
	}
	//max time_period in milli-seconds should be 134000
		//the time_period is in milli-seconds
		//generate time delay with normal mode
		//first calculate the number of ticks to get the required delay
	porti=port;
	pini=pin;
	unsigned int ticks;       //0-65536 ticks
	ticks=floor(0.489*(time_period));  //duty cycle is assumed to be 50 percent 
	
	ticks=ticks+1;    //round-off the ticks to greatest integer
	//ticks=65536-ticks; //the TCNT value
	temp=65536-ticks;
	TCCR1A=0x00;
	TCCR1B|=(1<<CS12)|(1<<CS10);                        //pre-scaler of 1024 is set 
	TCNT1=temp;
	TIMSK|=(1<<TOIE1);                       //enable the timer1 overflow flag 
    return temp;
}
unsigned int* generate_square_fast_pwm(unsigned int time_period,unsigned char duty_cycle,unsigned char port,unsigned char pin)
{
	//fast PWM mode generate wave with given time_period and duty cycle 
	//according to the PORT and the PIN and initialize them to LOW 
	array_new[0]=time_period;// to return the time_period and different duty cycle
	array_new[1]=duty_cycle;
	array_new[3]=pin;
	if(port=='A')
	{
		array_new[2]=1;
		DDRA|=(1<<pin);
		PORTA&=~(1<<pin);
	}
	else if(port=='B')
	{
		array_new[2]=2;
		DDRB|=(1<<pin);
		PORTB&=~(1<<pin);
	}
	else if(port=='C')
	{
		array_new[2]=3;
		DDRC|=(1<<pin);
		PORTC&=~(1<<pin);
	}
	else if(port=='D')
	{
		array_new[2]=4;
		DDRD|=(1<<pin);
		PORTD&=~(1<<pin);
	}
	sei();
	DDRD|=(1<<PD5);     //The OC1A pin has to be configured as an output for generation of waves
	//BOTTOM. In noninverting	//Compare Output mode, the Output Compare (OCnx) is cleared on the Compare Match between
	//TCNTn and OCRnx, and set at BOTTOM. In inverting Compare Output mode output is set on Compare
	//Match and cleared at BOTTOM. 
	//Here we generate the wave using the overflow flag by computing the TOP value using the OCR1A register
	
	TCCR1A|=(1<<WGM11)|(1<<WGM10);     // for the fast pwm overflow mode top defined using the OCR1A register 
	TCCR1B|=(1<<WGM12)|(1<<WGM13)|(1<<CS12)|(1<<CS10); //for 1024 duty cycle 
	OCR1A=floor((0.9765625)*(duty_cycle)*(time_period)/100); // initially provided the time period is specified in miliseconds 
	//OCR1A=30;
	TCNT1=0x0000;
	TIMSK|=(1<<TOIE1);       //enable the overflow flag 
	//now transfer the control to the ISR for further computations 
	return array_new;
}
unsigned int*  generate_square_ctc(unsigned int time_period,unsigned int duty_cycle)
{
	//generate the square wave with given duty cycle and time period using timer_1 and CTC mode 
	//the top value is adjusted using the OCR1A register 
	sei();
	DDRD|=(1<<PD5);
	DDRB=0xFF;
	array[0]=time_period;
	//pnt=pnt+1;
	array[1]=duty_cycle;
	//pnt=pnt-1;
	TCCR1A|=(1<<COM1A0);                      //toggle the OC1A pin on compare match 
	TCCR1B|=(1<<WGM12)|(1<<CS12)|(1<<CS10);   //set the pre-scalers to 1024 , enable the CTC mode 
	OCR1A=floor(0.976*duty_cycle*time_period/100); //for the high part of the wave : and the duty cycle in percentage and time_period in milliseconds 
	//OCR1AL=floor(0.976*duty_cycle*time_period/100);
	TIMSK|=(1<<OCIE1A);                        //OCIE1A bit set so that the interrupt is enabled 
	
	return array;                                // return the pointer to the time_period and duty_cycle
}
void read_frequency()         
{
	//The Input Capture is updated with the counter (TCNT1) value each time an event occurs on the ICP1 pin
	//(or optionally on the Analog Comparator output for Timer/Counter1). The Input Capture can be used for
	//defining the counter TOP value. We can use the CTC mode for the interrupt trigger for the same 
	//read the input frequency on the ICP1 pin
	//everything is done without the input capture noise canceler 
	sei(); // set global interrupt enable
	TCCR1A=0x00;          //the mode is Input Capture Mode
	TCCR1B|=(1<<ICES1);     //pos-edge is used as the capturer
	TCCR1B|=(1<<CS12)|(1<<CS10);    //for 1024 pre-scaler 
	TCNT1=0x0000;            //initialize just to take necessary precautions 
	TIMSK|=(1<<TICIE1);      //Input capture enable bit is set to ensure interrupt triggering 
	//read the ICR1 register , and subtract the two consecutive values to get pulse width (half time period)
	TCCR1B|=(1<<ICNC1);
}
/*
ISR(TIMER1_OVF_vect)
{
	cli();
	//the timer has successfully overflown 
	//toggle the Pin 
		//TCNT1=temp;
		TCNT1=0xFF00;
		PORTA^=(1<<PA0);
		if(porti=='A')
		{
			PORTA^=(1<<pini);
		}
		else if(porti=='B')
		{
			PORTB^=(1<<pini);
		}
		else if(porti=='C')
		{
			PORTC^=(1<<pini);
		}
		else if(porti=='D')
		{
			PORTD^=(1<<pini);
		}
		sei();
}

 */