/*
 * USART_header.c
 *
 * Created: 14-05-2018 13:00:37
 *  Author: Bhushan Deo
 */ 
/*Control the baud rate , frame select , synchronous or asynchronous mode , master or slave in synchronous modes , */
/*Data would be transmitted in standard 8 bit mode.*/
/*Parity and number of stop bits could be selected.*/
/*This is the source file for the header file.*/
#define F_CPU 8000000    
#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>
#include"USART_header.h"
#include<math.h>
#include<stdbool.h>
/*All the settings of the baud-rate are applied with 8MHz as the clock source frequency , consult the developer for changes in the same.*/
void initialize_USART(unsigned int baudrate,unsigned char mode,unsigned char stop,unsigned char parity0,unsigned char parity1,unsigned char tr)
{
	sei();
	/*UCSRC|=(1<<URSEL)|(1<<UCSZ1)|(1<<UCSZ0);                       // 8 bit data , and URSEL on for writing to UCSRC register
	UBRRL=0x33;       //for 9600 baud rate at 1MHz
	UCSRB&=~(1<<UDRIE);       //disable the interrupt UDRE interrupt , because this is polling based communication method 
	UCSRB|=(1<<TXEN)|(1<<RXCIE)|(1<<TXCIE);         //setup as transmitter , enable the interrupts
	//UCSRB|=(1<<UDRIE);            //enable the interrupt to begin writing*/
	
	/*The URSEL is used for writing either to the UBRRH or UCSRC register.*/
	/*Baud-Rate , Synchronous or Asynchronous mode , Number of start or stop bits , which parity to be used */
	/*Baud Rate with respect to the 8MHz case , error prone .*/
	/*Baud Rate should be in the standard multiples of 2400.*/
	unsigned char div = floor(baudrate/2400);   //what multiple of the baudrate
	UBRRL= ((int)floor((207/div)))&(0xFF);      //the low byte
	//UBRRL=0x33;
	/*Check the UBRR register to see what baud rate is generated.*/
    /*	Now set receiver or transmitter*/
    //sei();                                            //Enable global interrupts 
    
	/*Now go for the mode of operation .*/
	UCSRC|=(1<<URSEL)|(1<<UCSZ0)|(1<<UCSZ1);    //for writing into the UCSRC register in 8 bit mode 
	UCSRC|=(1<<URSEL)|(mode<<UMSEL);            //If mode==0 , asynchronous operation , else synchronous operation 
	
	/*Now go for parity selection .*/
	/*disabled for (0,0) , even for (0,1) odd for (1,1) */
    //for the parity bits and 8 bit data transfer communication and the stop 
	UCSRC|=(1<<URSEL)|(parity0<<UPM0)|(parity1<<UPM1);    //parity bit setting 
	UCSRC|=(1<<URSEL)|(stop<<USBS);          //number of stop bits 
	//UBRRH = ((int)floor((207/div)))>>8;         // the high byte
	//UBRRH|=(1<<URSEL);
	
	if(tr)
	{
		//transmitter enable
		UCSRB|=(1<<TXEN);        //transmit enable
	    UCSRB&=~(1<<UDRIE);      //Disable the ISR because this is polling 
	}
	else
	{
		UCSRB|=(1<<RXEN);         //receive enable
		UCSRB&=~(1<<RXCIE);       //Disable the reception complete interrupt
		UCSRB&=~(1<<UDRIE);
	} 
}
void send_data(unsigned char c)
{
	//_delay_ms(1000);
	//unsigned char o=0;
	UCSRB|=(1<<TXEN);
	char flag=0x01;
	//UCSRB|=(1<<TXEN);
	//Send the data , initialization is already done , by checking the UBRR register
	while ((UCSRA & (1 << UDRE)) == 0) {};
	    //initially UDRE flag would be set , once the data is being written to the UDR then it is automatically cleared
	//UDR=c;
	//_delay_ms(1000);
	//UCSRA&=~(1<<UDRE);
	//UCSRB&=~(1<<TXEN);
	/*while(flag>0)
	{*/
		//PORTB^=(1<<PB0);
		UDR=c;                   //UDRE flag would automatically be cleared
		//UCSRB&=~(1<<TXEN);
		//flag--;
		//c=0x00;
		UCSRB&=~(1<<TXEN);
		
	/*}*/
	//flag=1;          //only for one time transmission
	
	
}
unsigned char receive_data()
{
	while(!(UCSRA&(1<<RXC))); //Flag is set when there are unread data in the UDR register , else the flag is cleared
	return UDR;
}