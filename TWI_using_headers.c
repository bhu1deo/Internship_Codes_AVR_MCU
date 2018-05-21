/*
 * TWI_using_headers.c
 *
 * Created: 21-05-2018 11:04:20
 *  Author: Bhushan
 */ 

#define F_CPU 1000000
#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>
#include"TWI_header.h"
//Test the TWI function using the header files 
//This testing is done using the MT-master transmit mode 
int main(void)
{
	 
	DDRA=0xFF;
    //Master receive mode upload slave transmit in other device 
	unsigned char garbage;
	garbage = TWIcomm('M',0b0000001,0xA3,'S');  //data carries no relevance here
	PORTA=garbage;
    while(1)
    {
        //TODO:: Please write your application code 
    }
}