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
#include<stdlib.h>
#include<math.h>
#include<stdbool.h>
unsigned char gen_number(unsigned char );
void UART_init();
unsigned char UART_read();
unsigned char crc_calc(unsigned char* , unsigned char );
void UART_transmit(unsigned char);
unsigned char calc_leading_zeros(unsigned char );
unsigned char crc_string = 0xA7;   //The generator string used to calculate the CRC , ensure the data is greater than the key
unsigned char crc_verify(unsigned char* , unsigned char , unsigned char);
int main(void)
{
	UART_init(); 
	unsigned char data,*ptr,node_id_lsb,node_id_msb,func_code,length,init_addr;
	//crc_calc(0xFF);
	//UART_transmit(calc_leading_zeros(0x00));
	node_id_msb = 0x00;
	node_id_lsb = 0x03;
	func_code   = 0x08; 
	length      = 0x01;
	init_addr   = 0x01;
	data        = 0x40;
	//85 07 00 01 21 44 67 AA FF C1
	ptr = (unsigned char *)malloc((10)*sizeof(unsigned char)); //Memory is allocated already don't use & to address additionally
	*(ptr) = 0x85;
	*(ptr+1) = 0x07;
	*(ptr+2) = 0x15;
	*(ptr+3) = 0x05;
	*(ptr+4) = 0x21;
	*(ptr+5) = 0x44;
	*(ptr+6) = 0x67;
	*(ptr+7) = 0xAA;
	*(ptr+8) = 0xFF;
	*(ptr+9) = 0xC1;
	//UART_transmit(ceil((float)0/8));
	data            = crc_calc(ptr,9);
	UART_transmit(crc_calc(ptr,9));
	UART_transmit(crc_verify(ptr,data,9));
	//UART_transmit(crc_verify(0x01,0x01));
	/*while(i<6)
	{
		UART_transmit(*(ptr+i));
		i++;
	}*/
/*  UART_transmit(ceil((float)5/8));*/
		
		
		
       /* //Write your application code here
		data = UART_read();
		UART_transmit(crc_calc(data));
		key = crc_calc(data);
		UART_transmit(crc_verify(data,key));*/
    
}
unsigned char calc_leading_zeros(unsigned char data)
{
	//Calculate the leading zeros for the data byte 
	unsigned char number = 0;        //The number to be returned 
	unsigned char iter = 8;          //Maximum 9 iterations as our relevant data is only worth 9 bits 
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

unsigned char crc_verify(unsigned char* ptr, unsigned char key,unsigned char size)
{
	//For the multi-byte CRC computation , 
	//If ceil(number%8) changes (increments) then take in the next byte 
	//Do this until the complete data packet is exhausted 
	//Keep a variable data_high and two variables data_low and data_low_new
	//Append 7 zeros at the end as we are calculating CRC-8
	//Store the base data in data_ptr and access successive values using *(ptr+i)
	//First try with 16 bit then 32 bit and finally 48 bit data streams
	//Size is the total bytes of the available data 
	//First it is done for 2 bytes that is 16 bits
	//crc_string = 0b10100111
	//Calculate and return the CRC of the data
	//Append 7 zeros to the end as the string involved is 8 bit (1-byte)
	key  = key<<1;        //left shift the key as we want only the 7 lsg bits 
	unsigned char exhausted = 0x00;
	unsigned char data_high = *(ptr);   //The first byte 
	unsigned char data_low  = *(ptr+1); //The next byte in contention
	unsigned char count,number2,count_new;
	unsigned char number = 0;
	unsigned char old_ceil,new_ceil;
	//Compute the iterations only until they are less than the specified size
	while(number<(size*8+7))
	{
		//UART_transmit(floor((float)number/8));
		if(number==0)
		{
			old_ceil = 1;
		}
		else
		{
			old_ceil    = ceil((float)number/8);           //Old ceil value
		}
		
		
		//data_new = *(data_ptr+ceil((float)number/8));
		
		data_high = data_high^crc_string;
		//UART_transmit(data_high);
		count = calc_leading_zeros(data_high);
		number += count;
		//UART_transmit(number);
		new_ceil    = ceil((float)number/8);           //New ceil value , check for overflow
		
		//As soon as number becomes greater than size bytes , then take the zeros into consideration
		if((floor((float)number/8)>=size)&&(!exhausted))
		{
			number2   = number - count; //The previous value of the number variable
			count_new = 8*floor((float)number/8) - number2;
			data_high = data_high<<count_new;
			data_high = data_high | ((data_low&(gen_number(count_new)))>>(8-count_new));
			data_high = data_high<<(int)(number-8*floor((float)number/8));
			data_high = data_high | ((key&(gen_number(number-8*floor((float)number/8))))>>(int)(8-(number-8*floor((float)number/8))));
			key       = key<<(int)(number-8*floor((float)number/8));
			exhausted = 0x01;
			
		}
		else if((new_ceil!=old_ceil)&&(!exhausted))
		{
			//New ceil is not equal to the Old ceil that is number of zeros overflow
			//The current low data byte to be used and the next byte also to be used because the bits overflow
			number2   = number - count; //The previous value of the number variable
			count_new = 8*floor((float)number/8) - number2;
			data_high = data_high<<count_new;
			data_high = data_high | ((data_low&(gen_number(count_new)))>>(8-count_new)); //The current data_low value
			data_low  = *(ptr+(int)ceil((float)number/8));
			data_high = data_high<<(int)(number-8*floor((float)number/8));
			data_high = data_high | ((data_low&(gen_number(number-8*floor((float)number/8))))>>(int)(8-(number-8*floor((float)number/8)))); //The current data_low value
			data_low  = data_low<<(int)(number-8*floor((float)number/8));
		}
		else if((new_ceil==old_ceil)&&(!exhausted))
		{
			//The number of leading zeros don't overflow
			//Only the current low byte is enough
			data_high = data_high<<count;
			data_high = data_high | ((data_low&(gen_number(count)))>>(8-count)); //Only the current data-low to be used 
			data_low  = data_low<<count;                                 //Standard 
			
		}
		//Also check for the floor value not to exceed the total size 
		else if(exhausted)
		{
			//The floor value exceeds the total size , hence the complete data byte is exhausted 
			if((floor((float)number/8)<(size+1)))
			{
				data_high = data_high<<count;
				data_high = data_high | ((key&(gen_number(count)))>>(8-(count)));
				key       = key<<count;
			}
			else
			{
				number2   = number - count;
				data_high = data_high<<(size*8+7-number2);
				data_high = data_high | ((key&(gen_number(size*8+7-number2)))>>(8-(size*8+7-number2)));
			}
		}
		
		
		
		
		
		/*if((number>8) && (!exhausted))
		{
			number2   = number - count; //The previous value of the number variable
			count_new = 8 - number2;
			data_high = data_high<<count_new;
			data_high = data_high | ((data_low&(gen_number(count_new)))>>(8-count_new));
			data_high = data_high<<(number-8);
			exhausted = 0x01;
		}
		else if(!exhausted)
		{
			//Left shift and OR with the 2nd byte of the data involved
			data_high = data_high<<count;
			data_high = data_high | ((data_low&(gen_number(count)))>>(8-count));
			data_low  = data_low<<count; 
		}
		else if(exhausted)
		{
			if(number<16)
			{
				data_high = data_high<<count;
			}
			else
			{
				number2   = number - count;
				data_high = data_high<<(15-number2);
			}
		}
	}*/
	}
	return data_high<<1;
	/*For calculating 48 bit CRC , append 7 zeros to the 48 bits
	Then , keep appending the successive data till the last bit is exhausted
	Then use the zeros*/
}
unsigned char gen_number(unsigned char count)
{
	unsigned char data = 0x00;
	while(count>0)
	{
		data |= (1<<(8-count));
		count--;
	}
	return data;
}
unsigned char crc_calc(unsigned char* ptr,unsigned char size)
{
	
	
	//For the multi-byte CRC computation , 
	//If ceil(number%8) changes (increments) then take in the next byte 
	//Do this until the complete data packet is exhausted 
	//Keep a variable data_high and two variables data_low and data_low_new
	//Append 7 zeros at the end as we are calculating CRC-8
	//Store the base data in data_ptr and access successive values using *(ptr+i)
	//First try with 16 bit then 32 bit and finally 48 bit data streams
	//Size is the total bytes of the available data 
	
	
	
	
	//First it is done for 2 bytes that is 16 bits
	//crc_string = 0b10100111
	//Calculate and return the CRC of the data
	//Append 7 zeros to the end as the string involved is 8 bit (1-byte)
	unsigned char exhausted = 0x00;
	unsigned char data_high = *(ptr);   //The first byte 
	unsigned char data_low  = *(ptr+1); //The next byte in contention
	unsigned char count,number2,count_new;
	unsigned char number = 0;
	unsigned char old_ceil,new_ceil;
	//Compute the iterations only until they are less than the specified size
	while(number<(size*8+7))
	{
		//UART_transmit(floor((float)number/8));
		if(number==0)
		{
			old_ceil = 1;
		}
		else
		{
			old_ceil    = ceil((float)number/8);           //Old ceil value
		}
		
		
		//data_new = *(data_ptr+ceil((float)number/8));
		
		data_high = data_high^crc_string;
		//UART_transmit(data_high);
		count = calc_leading_zeros(data_high);
		number += count;
		
		new_ceil    = ceil((float)number/8);           //New ceil value , check for overflow
		
		//As soon as number becomes greater than size bytes , then take the zeros into consideration
		if((floor((float)number/8)>=size)&&(!exhausted))
		{
			number2   = number - count; //The previous value of the number variable
			count_new = 8*floor((float)number/8) - number2;
			data_high = data_high<<count_new;
			data_high = data_high | ((data_low&(gen_number(count_new)))>>(8-count_new));
			data_high = data_high<<(int)(number-8*floor((float)number/8));
			exhausted = 0x01;
			
		}
		else if((new_ceil!=old_ceil)&&(!exhausted))
		{
			//New ceil is not equal to the Old ceil that is number of zeros overflow
			//The current low data byte to be used and the next byte also to be used because the bits overflow
			number2   = number - count; //The previous value of the number variable
			count_new = 8*floor((float)number/8) - number2;
			data_high = data_high<<count_new;
			data_high = data_high | ((data_low&(gen_number(count_new)))>>(8-count_new)); //The current data_low value
			data_low  = *(ptr+(int)ceil((float)number/8));
			data_high = data_high<<(int)(number-8*floor((float)number/8));
			data_high = data_high | ((data_low&(gen_number(number-8*floor((float)number/8))))>>(int)(8-(number-8*floor((float)number/8)))); //The current data_low value
			data_low  = data_low<<(int)(number-8*floor((float)number/8));
		}
		else if((new_ceil==old_ceil)&&(!exhausted))
		{
			//The number of leading zeros don't overflow
			//Only the current low byte is enough
			data_high = data_high<<count;
			data_high = data_high | ((data_low&(gen_number(count)))>>(8-count)); //Only the current data-low to be used 
			data_low  = data_low<<count;                                 //Standard 
			
		}
		//Also check for the floor value not to exceed the total size 
		else if(exhausted)
		{
			//The floor value exceeds the total size , hence the complete data byte is exhausted 
			if((floor((float)number/8)<(size+1)))
			{
				data_high = data_high<<count;
			}
			else
			{
				number2   = number - count;
				data_high = data_high<<(size*8+7-number2);
			}
		}
		
		
		
		
		
		/*if((number>8) && (!exhausted))
		{
			number2   = number - count; //The previous value of the number variable
			count_new = 8 - number2;
			data_high = data_high<<count_new;
			data_high = data_high | ((data_low&(gen_number(count_new)))>>(8-count_new));
			data_high = data_high<<(number-8);
			exhausted = 0x01;
		}
		else if(!exhausted)
		{
			//Left shift and OR with the 2nd byte of the data involved
			data_high = data_high<<count;
			data_high = data_high | ((data_low&(gen_number(count)))>>(8-count));
			data_low  = data_low<<count; 
		}
		else if(exhausted)
		{
			if(number<16)
			{
				data_high = data_high<<count;
			}
			else
			{
				number2   = number - count;
				data_high = data_high<<(15-number2);
			}
		}
	}*/
	}
	return data_high;
	/*For calculating 48 bit CRC , append 7 zeros to the 48 bits
	Then , keep appending the successive data till the last bit is exhausted
	Then use the zeros*/
}