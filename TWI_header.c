/*
 * TWI_header.c
 *
 * Created: 21-05-2018 09:22:22
 *  Author: Bhushan
 */ 

#define F_CPU 1000000
#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>
#include"TWI_header.h"

unsigned char TWIcomm(unsigned char mode,unsigned char address,unsigned char data,unsigned char device)
{
	//Mode - M or S , Address - address of the slave , data - 8 bit data
	//In MT mode , the master transmits data in write mode , whereas the slave receives the data
	//In ST mode , the slave transmits data in write mode , the master receives the data
	//All data through the SDA , SCK lines only on the atmega32
	//Address has to be 7 bit only , one bit is reserved either for the read/write or for the general call address
	//For the write mode append a 0 at the end of the address in master 
	//For the slave append a 1 or a 0 to enable the general call address
	if(mode=='M')
	{
		//MT-SR mode address with write bit appended to the address 
		//check which mode
		if(device=='M')
		{
			//MT mode with the current device operating as a master
			//These variables are used to run the code only once repeated start conditions not allowed 
			unsigned char start_flag=0;
			unsigned char address_flag=0;
			unsigned char data_flag=0;
			//Bit Rate register is kept as 0 so that fclk/16 is the I2C frequency
			TWBR=0x00;
			TWCR=(1<<TWINT)|(1<<TWSTA)|(1<<TWEN);         //clear the interrupt flag to avoid running of the ISR , set the start bit to transmit the start bit
			TWSR=0x00;
			while(1)
			{
				if((TWSR==0x08)&&(start_flag==0))  //Start bit has been received
				{
					start_flag=1;
					TWDR=address<<1;  // To transmit the address and a zero appended at the msb in order to enable write mode
					TWCR=(1<<TWINT)|(1<<TWEN);  //clear the flag to transmit
					_delay_ms(10);
				}
				else if((TWSR==0x18)&&(address_flag==0))  //Address has been received
				{
					address_flag=1;
					TWDR=data;  //send the data
					TWCR=(1<<TWINT)|(1<<TWEN);    //clear the flag to transmit
					_delay_ms(10);
				}
				else if((TWSR==0x28)&&(data_flag==0))  //Data Byte has been received only then transmit the stop flag 
				{
					data_flag=1;
					//transmit the stop bit
					TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
					_delay_ms(10);
					return 0;  //return 0 if operating as a MT or ST
				}
				
			}
		}
		else if(device=='S')
		{
			//unsigned char flag=0;
			//sei();
			unsigned char temporary;
			TWAR=address<<1|0x01;                 //enable the General call address as well
			TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWEA);        //just enable the slave I2C communication and the address enable
			//PORTA=0x10;
			while(1)
			{
				//TODO:: Please write your application code
				if((TWSR==0x60)) //That is the SLA+W has been successfully acknowledged
				{
					//flag=1;
					TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWEA);
				}
				else if(TWSR==0xA0) //stop bit is received
				{
					temporary=TWDR; /*display data on PORTA*/
					//clear the flag
					TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWEA);
					return temporary;
				}
			}
		}
	}
	else if(mode=='S')
	{
		//That is MR or ST mode
		if(device=='M')
		{
			//The device operates as a MR mode
			unsigned char start_flag=0;
			unsigned char address_flag=0;
			//unsigned char stop_flag=0;
			unsigned char data_flag=0;
			
		    TWBR=0x00;        // 16 times the clock frequency
		    TWSR=0x00;        // Reset the TWSR to prevent reading of wrong values
		    TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWSTA);//master has to acknowledge to the slave when it has received the data so enable TWEA after address is sent and check the TWSR after the data is received
		     while(1)
		     {
			     if((TWSR==0x08)&&(start_flag==0))
			     {
				     //start condition has been transmitted
				     start_flag=1; //run only once
				     //now go for SLA+R
				     TWDR=address<<1|0x01;   //The 7 bit address is appended with the last bit being 1 for Read Mode
				     TWCR=(1<<TWINT)|(1<<TWEN);         //Clear the TWINT bit so as to begin transmission slave will give an acknowledge of the address
				     //_delay_ms(1000);
			     }
			     else if((TWSR==0x40)&&(address_flag==0))
			     {
				     //Address + Read bit is received now to receive the data we have to enable the acknowledge enable bit
				     address_flag=1;
				     TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWEA);  //Enable the enable acknowledge , so that when a data byte is received then it sends ack signal to the slave
				     //_delay_ms(1000);
			     }
			     else if((TWSR==0x50)&&(data_flag==0))
			     {
					 unsigned char temporary2;
				     //The data has been successfully acknowledged
				     temporary2=TWDR;
				     data_flag=1;
				     TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWSTO);       //Again clear the TWINT bit so as to enable for future transmissions and transmit the stop bit
				     //_delay_ms(1000);
					 return temporary2;  //Return the data received 
			     }
		     }
		}
		else if(device=='S')
		{
			//This device operates as a slave in the slave transmitter mode 
			TWAR=address<<1|0x01; //Enable general call address for this slave
			TWBR=0x00;
			TWSR=0x00;
			TWCR=(1<<TWEN)|(1<<TWEA)|(1<<TWINT);        //Enable TWI , enable Enable Acknowledge , clear TWINT
			/*Now firstly start bit will be received and acknowledgment can be seen at the master */
			/*Then SLA+R would be put on SDA check on the slave side*/
			/*If yes put data in the TWDR and then clear the flags */
			/*then acknowledgment will be sent from the master and then clear the flags to check for the stop condition */
			while(1)
			{
				if((TWSR)==0xA8)      // that is SLA+R is received and an ACK bit is transmitted
				{
					TWDR=data;  // The data to be transmitted
					TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWEA);  //clear the flags
					_delay_ms(10);
				}
				else if((TWSR)==0xB8)
				{
					//That is Data is transmitted and Ack has been received
					TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWEA);    //Now just wait for the stop bit which can't be read from the slave TWSR
					_delay_ms(10);
					return 0;
				}
			}
		}
	}
//No need to return anything 
}