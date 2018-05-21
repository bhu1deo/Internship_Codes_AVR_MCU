/*
 * SPI_transmit_using_headers.c
 *
 * Created: 15-05-2018 14:36:00
 *  Author: Bhushan
 */ 

#define F_CPU 1000000
#include<avr/io.h>
#include<math.h>
#include<util/delay.h>
#include<avr/interrupt.h>
#include"SPI_header.h"
/*This is only the master mode code slave mode is done separately*/
int main(void)
{
	DDRA=0xFF;
	unsigned char temp=0;
	initialize(1,'B',0);//configured as master 
	//_delay_ms(1000);
	
	while(temp<100)
	{
		_delay_ms(1000);
		PORTA=send_read_data(temp,1,'B',0); //put 100 as the char data in master mode
	    temp++;	
	}
	
    while(1)
    {
        //TODO:: Please write your application code 
    }
}