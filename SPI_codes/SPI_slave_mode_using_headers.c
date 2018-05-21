/*
 * SPI_receive_using_headers.c
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
/*This is only the slave mode code master mode is done separately*/
int main(void)
{
	DDRA=0xFF; //Read the data on PORTA
	unsigned char temp=100;
	initialize(0,'D',0);    //configured as slave , the D , 0 dont matter
	//_delay_ms(1000);
	while(temp>0)
	{
		//while(PINB&(1<<PB4));  //wait until the SS pin goes low 
		PORTA=send_read_data(temp,0); //put 100 as the char data in slave mode
	    temp--;	
	}
	
    while(1)
    {
        //TODO:: Please write your application code 
    }
}