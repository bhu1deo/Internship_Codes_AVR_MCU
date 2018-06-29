/*
 * CRC_using_atmega32.c
 *
 * Created: 29-06-2018 10:45:35
 * Author: Bhushan Deo for Embed-Gallery Electronics Services
 * This code is mainly meant for calculating the CRC and checking it and retrieving the original data using that CRC
 * 2 bytes : Data , CRC . 
 * Read the data from the UART port , generate a key , and check again with the key and display the result (should be zero)
 * The Data's MSB is made 1
 */ 

#define F_CPU 11059200
#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>
void UART_init();
unsigned char UART_read();
unsigned char crc_calc(unsigned char);
void UART_transmit(unsigned char );
unsigned char calc_leading_zeros(unsigned char);
unsigned char crc_string = 0b00001101;   //The generator string used to calculate the CRC
unsigned char crc_verify(unsigned char, unsigned char);
int main(void)
{
	UART_init();
	unsigned char data,key;
	//crc_calc(0xFF);
    while(1)
    {
        //TODO:: Please write your application code 
		/*data = UART_read(); //The data from the UART
		UART_transmit(data);
		UART_transmit(calc_leading_zeros(data));
		Checked that this code actually works
		*/
		//Read 7 bit data from the UART and calculate its CRC , 1 is appended to its MSB and display on the UART
		data = UART_read();        //Read the virtual port data
		key = crc_calc(data);
		UART_transmit(key);
		UART_transmit(crc_verify(data,key));
    }
}
unsigned char calc_leading_zeros(unsigned char data)
{
	//Calculate the leading zeros for the data 
	unsigned char number = 0;        //The number to be returned 
	unsigned char iter = 4;          //Maximum 4 iterations as our relevant data is only worth a nibble 
	while((!(data&0x80))&(iter>0))
	{
		iter--;
		number++;
		data = data<<1;
	}
	return number;
}
void UART_init()
{
	//Initialize the UART
	UCSRC=0;
	UBRRH=0x00;
	UBRRL=0x47;         //storing lower bits here , at 11059200
	UCSRB=1<<RXEN|1<<TXEN;		//enable receiver mode only
	UCSRC=1<<URSEL|1<<UCSZ1|1<<UCSZ0;		//transmitting 8 bit data
}
unsigned char UART_read()
{
	while(!(UCSRA&(1<<RXC)));
	return UDR;
}
void UART_transmit(unsigned char data)
{
	//Transmit the data character
	while(!(UCSRA&(1<<UDRE)));		//loop will run till the buffer is empty
	UDR=data;		//then complete data is copied to UDR
}
unsigned char crc_verify(unsigned char data, unsigned char key)
{
	//Calculate and return the CRC of the data
	data = data | 0x80;       //Make the MSB 1
	unsigned char low_byte,high_byte,count,number2;
	high_byte = data&0xF0;
	low_byte = (((data&0x0F)<<4)|(key>>4));
	unsigned char number=0;
	while(number<8)
	{
		high_byte = high_byte^(crc_string<<4);
		
		count = calc_leading_zeros(high_byte);
		number += count;
		
		if(number>7)
		{
			number2=number-count;
			high_byte=high_byte<<(7-number2);
		}
		else
		{
			high_byte = high_byte<<count;
		}
		
		
		/*UART_transmit(number);*/
		while(count>0)
		{
			
			if(low_byte&0x80)
			{
				high_byte |= (1<<(3+count));
			}
			low_byte = low_byte<<1;
			count--;
		}
		
	}
	//UART_transmit(high_byte);
	return high_byte<<1;
}
unsigned char crc_calc(unsigned char data)
{
	//Calculate and return the CRC of the data
	data = data | 0x80;       //Make the MSB 1
	unsigned char low_byte,high_byte,count,number2;
	high_byte = data&0xF0;
	low_byte = ((data&0x0F)<<4);
	unsigned char number=0;
	while(number<8)
	{
		high_byte = high_byte^(crc_string<<4);
		
		count = calc_leading_zeros(high_byte);
		number += count;
		
		if(number>7)
		{
			number2=number-count;
			high_byte=high_byte<<(7-number2);
		}
		else
		{
			high_byte = high_byte<<count;
		}
		
		
		/*UART_transmit(number);*/
		while(count>0)
		{
			
			if(low_byte&0x80)
			{
				high_byte |= (1<<(3+count));
			}
			low_byte = low_byte<<1;
			count--;
		}
		
	}
	//UART_transmit(high_byte);
	return high_byte<<1;
}