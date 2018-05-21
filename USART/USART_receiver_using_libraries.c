/*
 * USART_receiver_using_libraries.c
 *
 * Created: 21-05-2018 17:48:57
 *  Author: Bhushan
 */ 

#define F_CPU 8000000
#include<avr/io.h>
#include<util/delay.h>
#include<avr/interrupt.h>
#include"USART_header.h"
int main(void)
{
	DDRA=0xFF;
	initialize_USART(9600,0,0,0,0,0);
	while(1)
	{
		
		PORTA=receive_data();
	}
    
}