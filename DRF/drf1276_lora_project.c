/*
 * GccApplication1.c
 * Code written for Atmega32 MCU only
 * Created: 27-06-2018 12:00:08
 * Author: Bhushan Deo
 * Created for the Farm Automation Project
 * Include proper GSM/WiFi code for altering the configurations 
 * Configurations can be altered with these functions , and are set to default for TESTING purposes ONLY
 */ 

/*This is the code written for the atmega32 to interface with the LORA drf1276DM modules
In order to configure the parameters, the EN pin should be connected to logic low in any mode. Both the ENABLE and the SET should be LOW for express communication
RF_Factor is configured as default to be 128 that is 0x07, similarly the RF_BW is configured as a default : 125kHz or 0x07 , RF_POWER set to maximum of 20dBm 0x07
Breath and wake are configured as default 2seconds and 32ms respectively*/


/* ------------------------------Comments------------------------------------------------------------*/
/*//The virtual terminal is the monitor corresponding to the atmega32
//Rx of the UART to the Rx of the MCU
//Tx of the UART to the Tx of the MCU
//Aux is used as an interrupt pin , see datasheet for details 
//Whenever data is received then the MCU can start reading the data via the UART interface 
//This code is configurable for both central and node modules
//The node module must send it's node-Id , so that the central node can know which node is sending the data 
//The data is displayed on the UART in this code along with storage in the ptr
//Enable is PB1 
//Set is PB0
//Aux is PB2 - The INT2 interrupt 
//The frame format is as follows : Node ID (2 bytes) + Function Code(1byte) + Length(How many addresses to be read/written) + Base-Address(1byte) + Data(represented by length bytes) + CRC-key(1byte)
//The node-Id is not involved in CRC calculation 
//The function code has MSB as 1 , so that CRC computation follows 
//Here the AUX pin has indicated that the data reception has begun
//Thus the MCU can start reading the data via the UART interface
//No need of End of Data String as the frame format is specified and the CRC is matched
//Due to the very good AUX pin , we can use interrupt to begin receiving data when the module is ready
//According to the frame format the string is (5+L) bytes with one additional byte for CRC
//Once the length parameter is read , then we can allocate memory for the rest of the bytes
//The node automatically strips off the node-Id , whilst the central does not
//The MSB of the function code should be 1 for CRC
//On the central node side : The user needs to strip off the node-Id received
//Node side : 2nd byte is length byte
//This illustrative code is written for Node side module only 
*/


/* ------------------------------MACROS------------------------------------------------------------*/

#define F_CPU			  11059200
#define ENABLE            PB1
#define SET               PB0
#define AUX				  PB2 
#define INT_PIN           PB2
#define BAUD_1200         0x01
#define BAUD_2400         0x02
#define BAUD_4800         0x03
#define BAUD_9600         0x04
#define BAUD_19200        0x05
#define BAUD_38400        0x06
#define BAUD_57600        0x07
#define PARITY_NONE       0x00
#define PARITY_ODD        0x01
#define PARITY_EVEN       0x02
#define FREQ_866		  0x01             //D88024
#define FREQ_915		  0x02             //E4C026
#define FREQ_433		  0x03             //6C4012
#define MODE_STANDARD	  0x00
#define MODE_CENTRAL	  0x01
#define MODE_NODE		  0x02
#define EOD               0xFF


/* ------------------------------Libraries------------------------------------------------------------*/

#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>
#include<stdlib.h>
#include<stdbool.h>

/* ------------------------------Functions-And-Globals------------------------------------------------------------*/

void DRF_read();                     //read the configurations of the module 
void DRF_configure(unsigned char, unsigned char , unsigned char , unsigned char , unsigned char, unsigned char, unsigned char );  // node-Id , network-Id 
void UART_transmit(unsigned char);
void UART_init();
void interrupt_init();                          //Initialize the Interrupt Routine 
void verify(unsigned char* ,unsigned char, unsigned char);  //To process the received data 
unsigned char crc_verify(unsigned char* ,unsigned char, unsigned char);  
void decode(unsigned char*,unsigned char);
void read_from_node(unsigned char* ,unsigned char );
void write_to_node(unsigned char* );
void write_to_central(unsigned char* ,unsigned char );
unsigned char calc_leading_zeros(unsigned char data);
unsigned char gen_number(unsigned char count);
unsigned char UART_read();
unsigned char number = 0;                 //Number of bytes of the data received 
unsigned char* ptr;                   //To read the received data
unsigned char* ptr2;
unsigned char read = 0;                       //0 signifies that the data that is read is processed , 1 signifies to be processed
unsigned char crc_string = 0xA7;
unsigned char nodeidlsb,nodeidmsb,networkid,crc;  //The node and the network id , node ID is 2 bytes , network Id is 1 byte , central node doesn't have a node Id


/* ------------------------------Main-Code------------------------------------------------------------*/

int main(void)
{
	sei();
	nodeidmsb = 0x00;          //Change according to your convenience
	nodeidlsb = 0x02;          //Change according to your convenience
	networkid = 0x01;          //Change according to your convenience
	_delay_ms(1000);
	interrupt_init();          //Initialize the external-interrupt
	UART_init();
     
	DRF_configure(BAUD_9600,PARITY_NONE,FREQ_866,MODE_NODE,nodeidmsb,nodeidlsb,networkid);
	//This is the node mode's code 
	//For testing request the central to send the data at periodic intervals 
	//AUX pin connected to the specific INT pin of the processor 
		
    while(1)
    {
        //data=UART_read();
        /*_delay_ms(1000);
        UART_transmit(0x00);
        UART_transmit(0x02);
        UART_transmit(0xAA);
		UART_transmit(0xFF);
		UART_transmit(0xEB);*/
    }
}
/* ------------------------------User-Defined-Functions------------------------------------------------------------*/

void interrupt_init()
{
	//Initialize the interrupt
	//Therefore, it is recommended to first disable INT2 by clearing its Interrupt Enable bit in the GICR Register.
	//Then, the ISC2 bit can be changed. 
	//Finally, the INT2 Interrupt Flag should be cleared by writing a logical one to its Interrupt Flag bit (INTF2) in the GIFR Register before the interrupt is re-enabled.
    
	MCUCR = (1<<ISC01);     //Rising edge
	GICR |= (1<<INT0);      //re-enable the interrupt
	
}


void DRF_configure(unsigned char baud, unsigned char parity, unsigned char freq, unsigned char mode, unsigned char nodeidmsb,unsigned char nodeidlsb, unsigned char networkid)
{
	//Transmit the Default bytes along with calculating the CRC for the total payload before the CRC
	unsigned char array[25];              //The array will store all the values of the data to be sent 
	unsigned char sum=0; 
	unsigned char i=0;                  
	array[0]=0xAF;
	array[1]=0xAF;
	array[2]=0x00;
	array[3]=0x00;
	array[4]=0xAF;
	array[5]=0x80;
	array[6]=0x01;               //write mode
	array[7]=0x0E;               //length of the string 
	array[8]=baud;
	array[9]=parity;
	if(freq==0x01)
	{
		array[10]=0xD8;
		array[11]=0x80;
		array[12]=0x24;
	}
	else if(freq==0x02)
	{
		array[10]=0xE4;
		array[11]=0xC0;
		array[12]=0x26;
	}
	else 
	{
		array[10]=0x6C;
		array[11]=0x40;
		array[12]=0x12;
	}
	array[13]=0x07;         //RF_Factor
	array[14]=mode;
	array[15]=0x07;          //RF_BW
	array[16]=nodeidmsb;
	array[17]=nodeidlsb;
	array[18]=networkid;
	array[19]=0x07;          //RF_POWER
	array[20]=0x00;          //Breath : 2 seconds
	array[21]=0x04;          //Waker
	//Now the CRC 
	while(i<22)
	{
		sum+=array[i];
		i++;
	}
	i=0;
	array[22]=sum%256;     
	array[23]=0x0D;
	array[24]=0x0A;
	//Now send the data on the UART PORT
	while(i<25)
	{
		UART_transmit(array[i]);
		i++;
	}
}
void DRF_read()
{
	//Read and transmit the read data on the UART
	unsigned char array[25];              //The array will store all the values of the data to be sent
	unsigned char sum=0;
	unsigned char i=0;
	array[0]=0xAF;
	array[1]=0xAF;
	array[2]=0x00;
	array[3]=0x00;
	array[4]=0xAF;
	array[5]=0x80;
	array[6]=0x02;               //write mode
	array[7]=0x0E;               //length of the string
	array[8]=0x00;
	array[9]=0x00;
	array[10]=0x00;
	array[11]=0x00;
	array[12]=0x00;
	array[13]=0x00;         //RF_Factor
	array[14]=0x00;
	array[15]=0x00;          //RF_BW
	array[16]=0x00;
	array[17]=0x00;
	array[18]=0x00;
	array[19]=0x00;          //RF_POWER
	array[20]=0x00;          //Breath : 2 seconds
	array[21]=0x00;          //Waker
	//Now the CRC
	while(i<22)
	{
		sum+=array[i];
		i++;
	}
	i=0;
	array[22]=sum%256;
	array[23]=0x0D;
	array[24]=0x0A;
	//Now send the data on the UART PORT
	while(i<25)
	{
		UART_transmit(array[i]);
		i++;
	}
	//UCSRB=1<<RXEN;
}
void UART_init()
{
	//Initialize the UART
	UCSRC=0;
	UBRRH=0x00;
	UBRRL=0x47;         //storing lower bits here , at 11059200
	UCSRB=1<<TXEN|1<<RXEN;		//enable receiver and transmitter
	UCSRC=1<<URSEL|1<<UCSZ1|1<<UCSZ0;		//transmitting 8 bit data
}
void UART_transmit(unsigned char data)
{
	//Transmit the data character
	while(!(UCSRA&(1<<UDRE)));		//loop will run till the buffer is empty
	UDR=data;		//then complete data is copied to UDR
}
unsigned char UART_read()
{
	while(!(UCSRA&(1<<RXC)));
	return UDR;
}

/*-------------------------------------Reading process through the interrupt service routine-----------------------------*/

ISR(INT0_vect)
{
	unsigned char free_udr = UDR; //free the UDR by reading it's value and ensuring that no garbage is detected
	
	//Node side code , read data , and then also verify data then decode the function code encoded in the data 
	//Central sends a request (read or write to the node side) 
	//Mode returns the data or the acknowledgment 
	unsigned char i   = 0x00;
	unsigned char function_code , length , base_address ; 
		
		function_code = UART_read();
		length        = UART_read();
		base_address  = UART_read();
		ptr           = (unsigned char *)malloc((length+3)*sizeof(unsigned char)); //length bytes of data
		*(ptr)        = function_code;
		*(ptr+1)      = length;
		*(ptr+2)      = base_address;
		while(i<length) 
		{
			//Read the data-bytes , for the read_from_node operation , these bytes has to be zero
			*(ptr+i+3)= UART_read();
			i++;
		}
		crc= UART_read();      //The CRC received
		verify(ptr,crc,(length+3)); 
		free(ptr);             //free the pointer only when the operations on the data are completed 
}
void verify(unsigned char* pointer,unsigned char CRC,unsigned char size)
{
	//size stores the length of the bytes allocated to the pointer
	//The verify function takes one less size 
	//Verify the CRC
	//Pass the base address of the pointer and the CRC to verify
	if(!crc_verify(pointer,CRC,(size-1)))
	{
		//CRC is verified!!!!!!! , transmit the message of correct CRC , return single byte as 0x01
		//Decode the function code and proceed for further operations 
// 		DDRC          = (1<<PC7);
// 		PORTC         = (1<<PC7);
		decode(pointer,(size));    //Decode the function code
	}
	else
	{
		//Wrong CRC verification!!!!
		//Transmit the message of wrong crc , return single byte as 00
// 		DDRC          = (1<<PC7);
// 		PORTC         = 0x00;
	}
}
void decode(unsigned char* pointer,unsigned char len)
{
	//Decode the function code , only node side here , ack is considered implicit
	switch(*(pointer))
	{
		case 0x80 : /*Read from node*/ read_from_node(pointer,len); break;  //Read the sensor readings of the corresponding node      
		case 0x81 : /*Write to node*/ write_to_node(pointer); break;        //Write the actuator values for the corresponding node      
		default   : break;
	}
	
}
void read_from_node(unsigned char* pointer,unsigned char len)
{
	//The bytes of the data stream from the central should be zero these must be written to and then written 
	//Read from the node , and send the data to the central node , also send an acknowledgment request(implicit) ,  central should send an acknowledgment
	//The MCU should read the length bytes from init_address (sensor-data)
	//And then send the data to the central node and the central must send an ack to the node 
	//Keep sending till the ACK is received from the central node 
	//Read from the init_address and until the length number of bytes
	unsigned char length = *(pointer+1);
	unsigned char base_address = *(pointer+2);
	//Now using these two parameters , the user should put the data (eg. read sensor data) in the ptr from *(ptr+3) to *(ptr+length+2)
	//Send the read to the central node
	write_to_central(pointer,len); //User sends the read data to the central
}
//Also a function read from central to receive the current status of the corresponding node 
//
void write_to_central(unsigned char* pointer,unsigned char len)
{
	//Assuming that the pointer has already the data to be sent to the central node
	//Send the read data to the central node , 
	//First send the node-Id of the current node , then the data , so that the central node knows from where has the data originated 
	//Function code has to be write to central , length , base_address , crc has to be sent 
	UART_transmit(nodeidmsb);
	UART_transmit(nodeidlsb);
	*(pointer)      = 0x85;        //Function code to be written
	unsigned char i = 0x00;
	while(i<(len+2))
	{
		UART_transmit(*(pointer+i));
	}
	UART_transmit(crc);   //Transmit the CRC at the end 
}
void write_to_node(unsigned char* pointer)
{
   //Write the received data to the actuators starting from the init_address and up to length bytes
   //Send an acknowledgment to the central node as well
   unsigned char length = *(pointer+1);
   unsigned char base_address = *(pointer+2);
   //Now it is expected that the user puts the received data in to the actuator registers 
   //Send acknowledgment to the central , sending the node-id and 0x01
   UART_transmit(nodeidmsb);
   UART_transmit(nodeidlsb);
   UART_transmit(0x86);
   //Ack is sent the job is done 
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
	unsigned char exhausted = 0x00;
	unsigned char data_high = *(ptr);   //The first byte 
	unsigned char data_low  = *(ptr+1); //The next byte in contention
	unsigned char count,number2,count_new;
	unsigned char number = 0;
	unsigned char old_ceil,new_ceil;
	//Compute the iterations only until they are less than the specified size
	while(floor((float)number/8)<(size+1))
	{
		//UART_transmit(floor((float)number/8));
		if(!number)
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
			}
		}
	}
	return data_high<<1;
	/*For calculating 48 bit CRC , append 7 zeros to the 48 bits
	Then , keep appending the successive data till the last bit is exhausted
	Then use the zeros*/
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