/*
 * SPI_header.c
 *
 * Created: 15-05-2018 12:54:04
 *  Author: Bhushan
 */ 

#define F_CPU 1000000      //1MHz of clock frequency
#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>
#include"SPI_header.h"
#include<stdbool.h>
unsigned char porti;
unsigned char pini;
//SPCR , SPSR , SPDR are the only registers involved in the communication process 

void initialize(unsigned char ms ,unsigned char port , unsigned char pin )
{
	//Here the communication is carried out at standard fosc/4 but any of the frequencies as given in the datasheet can be adjusted 
	//initialize the communication  process 
	SPCR&=~(1<<SPIE);         //disable the SPIE interrupt enable in order to execute the polling process
	SPCR|=(ms<<MSTR)|(1<<SPE)|(1<<CPHA);  //rest are set to default 0 , see datasheet for details
	porti=port;
	pini=pin;
	if(ms)
	{
		//Master's slave select pin to be kept as output , MOSI output , SCK output , MISO input
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
		//MOSI output 
		DDRB|=(1<<PB5);          //MOSI to be kept as output
		//MISO input
		DDRB&=~(1<<PB6);          //MISO to be kept as input
		//SCK as output
		DDRB|=(1<<PB7);
		
	}
	//else the slave's slave select(assuming atmega32) to be kept as input , MISO as output , SCK as input , MOSI as input 
	else
	{
		//SS as input
		DDRB&=~(1<<PB4);
	    //MOSI as input 
		DDRB&=~(1<<PB5);
		//MISO output
		DDRB|=(1<<PB6);
		//SCK as input
		DDRB&=~(1<<PB7);
	}
	//Writing to the SPDR register initiates data transmission.
	//Reading the register causes the Shift
    //Register Receive buffer to be read and thereby clearing the SPIF flag thereafter 
}
unsigned char send_read_data(unsigned char data,unsigned char ms )
{
     //send and read the data corresponding to SPI communication
	 //This function also returns the unsigned char data (8 bits) to the main function 
	 unsigned char temp;   //To return the data read in the SPDR register
	 /*while(!(SPSR&(1<<WCOL)))
	 {
		 
	 }
	 temp=SPDR;*/
	 if(ms)
	 {
		 //Device operates as a master 
		 //select the slave by configuring the SS to low and thus selecting that particular slave 
		 if(porti=='A')
		 {
			 PORTA&=~(1<<pini);
		 }
		 else if(porti=='B')
		 {
			 PORTB&=~(1<<pini);
		 }
		 else if(porti=='C')
		 {
			 PORTC&=~(1<<pini);
		 }
		 else if(porti=='D')
		 {
			 PORTD&=~(1<<pini);
		 }
	 }
	 /*while(PINB&(1<<PB4))       //The SPIF flag would be cleared when the transmission process is over
	 {
		 //do nothing wait
	 }*/
	 SPDR=data;            //Load the data to initiate transmission (master side) else wait till SS goes low(slave side)
	 //Else if the device operates as a slave , then just wait
	 while(!(SPSR&(1<<SPIF)))       //The SPIF flag would be set when the transmission process is over 
	 {
		 //do nothing wait
	 }
	 //Transmission process is done , so SPIF flag is set and read the data in the SPDR data register to clesar the SPIF flag
	 temp=SPDR;
	 return temp;
}