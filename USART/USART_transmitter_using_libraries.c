/*
 * USART_using_libraries.c
 *
 * Created: 14-05-2018 16:42:17
 *  Author: Bhushan
 */ 
//USART transmission and reception using created libraries and header files 
#define F_CPU 8000000
#include<avr/io.h>
#include<util/delay.h>
#include<avr/interrupt.h>
#include"USART_header.h"
void send_data(unsigned char );
int main(void)
{
	DDRA=0xFF;
	signed char o=0;
	initialize_USART(9600,0,0,0,0,1);    // initialize the transmitter
	unsigned char str[]="Narkhede the topper";
	o--;
	DDRB|=(1<<PB0)|(1<<PB1);
	while(str[o]!=0)
	{  
		_delay_ms(100);
		o++;
		PORTA=o;
		send_data(str[o]);	
	}
	
	
    
}
