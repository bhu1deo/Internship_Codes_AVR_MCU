/*
 * Test_ADC.c
 *
 * Created: 21-05-2018 16:27:08
 *  Author: Bhushan
 */ 

#define F_CPU 1000000
#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>
#include"ADC_header.h"
int main(void)
{
	DDRB=0xFF;
	while(1)
	{
		PORTB=start_ADC_conversion(1, 0); /*5V reference with ADC0 connected as input*/
	}
}