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
Breath and wake are configured as default 2-seconds and 32-ms respectively*/


/*-------------------------------Comments------------------------------------------------------------*/
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
//According to the frame format the string is (3+L) bytes with one additional byte for CRC
//Once the length parameter is read , then we can allocate memory for the rest of the bytes
//The node automatically strips off the node-Id , whilst the central does not
//The MSB of the function code should be 1 for CRC
//On the central node side : The user needs to strip off the node-Id received which is the data received from that Node
//Central side : 2nd byte is length byte
//This illustrative code is written for Central side module only 
*/


/*-------------------------------MACROS------------------------------------------------------------*/

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
unsigned char crc_calc(unsigned char* ,unsigned char );
void decode(unsigned char*,unsigned char);
void read_from_node(unsigned char ,unsigned char ,unsigned char , unsigned char );
void write_to_node(unsigned char ,unsigned char ,unsigned char* ,unsigned char ,unsigned char );
void write_to_central(unsigned char* ,unsigned char );
void read_from_central(unsigned char* ,unsigned char );
unsigned char calc_leading_zeros(unsigned char data);
unsigned char gen_number(unsigned char count);
unsigned char UART_read();
unsigned char number = 0;                 //Number of bytes of the data received 
unsigned char* ptr;                   //To read the received data
//Process the data in the ISR as soon as it is received and free the pointer
unsigned char crc_string = 0xA7;          //The CRC generating string function
unsigned char nodeidlsb,nodeidmsb,networkid;  //The node and the network id , node ID is 2 bytes , network Id is 1 byte , central node doesn't have a node Id


/* ------------------------------Main-Code------------------------------------------------------------*/

int main(void)
{
	sei();
	nodeidmsb = 0x00;          //Change according to your convenience - Central Node ID
	nodeidlsb = 0x03;          //Change according to your convenience - Central Node ID 
	networkid = 0x01;          //Change according to your convenience - Central Network ID
	_delay_ms(1000);
	interrupt_init();          //Initialize the external-interrupt
	UART_init();
     
	//DRF_configure(BAUD_9600,PARITY_NONE,FREQ_866,MODE_CENTRAL,nodeidmsb,nodeidlsb,networkid);
	//This is the node mode's code 
	//For testing request the central to send the data at periodic intervals 
	//AUX pin connected to the specific INT pin of the processor 
		DDRC = (1<<PC6)|(1<<PC7);
    while(1)
    {
        //data=UART_read();
        
		
        /*UART_transmit(0x00);
        UART_transmit(0x02);
        UART_transmit(0xAA);
		UART_transmit(0xFF);
		UART_transmit(0xEB);*/
    }
}
/* ------------------------------User-Defined-Functions------------------------------------------------------------*/

void interrupt_init()
{
	//Assuming INT0 was used 
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


/*---------------------------------Read_from_node-and-Write_to_node-are-the-transmit-functions------------------*/

//Read sensor data from the node 
void read_from_node(unsigned char node_id_lsb , unsigned char node_id_msb, unsigned char len, unsigned char base_address)
{
	//After the command to read the data is sent the ISR reads the data received from the node
	//Read the data from the  Node , specified by its node-address , the data pointer must contain 00 ,we'll do it here rather than from the user
	//Size : The Actual Size of the data except the function code , base_address , nodeId or the CRC
	//Calculate the CRC and send it in the end
	//First specify the node_id of the specific node
	//Then the appropriate function code
	//Then the size and the initial address of the sensors to be read
	//Then transmit 0 bytes of data and finally the CRC
	unsigned char *ptr_new;       //used for calculating the CRC
	ptr_new   =   (unsigned char *)malloc((len+5)*sizeof(unsigned char)); //Already discussed
	*(ptr_new) =   node_id_msb; //node-id from where to read
	*(ptr_new+1) = node_id_lsb; //node-id from where to read
	*(ptr_new+2) = 0x80;        //The function code for the read-from-node operation is 0x80
	*(ptr_new+3) = len;         //How many bytes to read
	*(ptr_new+4) = base_address; //Base address of the sensor data
	UART_transmit(node_id_msb); 
	UART_transmit(node_id_lsb);
	UART_transmit(0x80);
	UART_transmit(len);
	UART_transmit(base_address);
	unsigned char i = 0x00;
	while(i<len)
	{   //5 to len+4
		*(ptr_new+5+i) = 0x00;           //Read the data so just transmit 0x00
		UART_transmit(0x00);
		i++;
	}
	UART_transmit(crc_calc(ptr_new,(len+4))); //Transmit the CRC at the end  , total (length+6) bytes have been transmitted
	
}

//Write the actuator data to the node
void write_to_node(unsigned char node_id_lsb , unsigned char node_id_msb , unsigned char* pointer , unsigned char size, unsigned char base_address)
{
	//Write the data to the node , specified by its node-address , the data to be written is contained in a pointer and it's size is size bytes
	//Size : The Actual Size of the data except the function code , base_address , nodeId or the CRC 
	//Calculate the CRC and send it in the end 
	//First specify the node_id of the specific node
	//What we'll do here is allocate all the memory dynamically using realloc of the std_lib in C
	//Ensure that the data to be written into the registers is written in reverse order before passing the arguments
	//Assuming that the data to be written is already starting from (ptr+0) to (ptr+size-1) and rest of the data is written as follows : 
	unsigned char *ptr_new;            //required for the CRC
	ptr_new   = (unsigned char *)realloc(pointer,(size+5)*sizeof(unsigned char));
	*(ptr_new+size)   = base_address;  //Base_address or init_address
	*(ptr_new+size+1) = size;          //length byte 
	*(ptr_new+size+2) = 0x81;          //function code for writing to the specified node
	*(ptr_new+size+3) = node_id_lsb;          //The size of the data bytes (number of data_bytes) to be written
	*(ptr_new+size+4) = node_id_msb;
    //Now transmit everything in reverse order , hence we have to ensure that the data bytes are to be entered in reverse order
	unsigned char i = 0x00;
	while(i<(size+5))
	{
		//i-> 0 to size+4 ; size+4 to 0 
		UART_transmit(*(ptr_new+(size+4-i)));
		i++;
	}
	UART_transmit(crc_calc(ptr_new,(size+4))); // size+4 not size+5 , see the crc function , CRC to be transmitted 
	//On the node side , the node-Id will be automatically filtered , CRC will be done , then the function code will be decoded
	//Along with the base address and the length bytes the length byte data will be written 
	//Then an acknowledgment will be sent by the node module
}
/*---------------------------Transmit functions are done-----------------------------------------------------*/










/*---------------------------------------------Reading using the ISR verify and decode------------------------*/
//Verify and generate functions are required both central as well as node side one while transmitting the data 
//One while receiving and verifying the data

ISR(INT0_vect)
{
	unsigned char free_udr = UDR; //free the UDR by reading it's value and ensuring that no garbage is detected
	
	//Central side code , read data , and then also verify data then decode the function code encoded in the data 
	//Node either sends the read data , or sends an acknowledgment , or asks for the data  
	//Central either writes or reads the Node-Data or sends an acknowledgment after receiving the read data from the node or returns the status of the current node
	unsigned char i   = 0x00;
	unsigned char function_code , length , base_address ,received_node_id_msb,received_node_id_lsb,crc;
	    /*UART_transmit('I');
		UART_transmit('S');
		UART_transmit('R');*/
		received_node_id_msb = UART_read();       //The node sends its ID while sending the data
		received_node_id_lsb = UART_read();       //No Id filtering is done on the central  side 
		function_code = UART_read();
		length        = UART_read();
		base_address  = UART_read();
		ptr           = (unsigned char *)malloc((length+5)*sizeof(unsigned char)); //length bytes of data
		*(ptr)        = received_node_id_msb;
		*(ptr+1)      = received_node_id_lsb;
		*(ptr+2)      = function_code;
		*(ptr+3)      = length;
		*(ptr+4)      = base_address;
		
		while(i<length) 
		{
			//i goes from 0 to length-1 : length bytes of data to be read
			//Read the data-bytes , for the read_from_node operation , these bytes has to be zero
			*(ptr+i+5)= UART_read();
			i++;
		}
		
		crc= UART_read();      //The CRC received
		
		verify(ptr,crc,(length+5)); //pass the actual size in this function , size-1 is taken care of 
		
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
		DDRC          = (1<<PC7);
 		PORTC         = (1<<PC7);        //  for verification
		//decode(pointer,(size));    //Decode the function code
	}
	else
	{
		//Wrong CRC verification!!!!
		//Transmit the message of wrong crc , return single byte as 00
      	DDRC          = (1<<PC7);
 		PORTC         = 0x00;                 // for verification 
	}
}
void decode(unsigned char* pointer,unsigned char len)
{
	//Decode the function code , here the function code is the 3rd byte
	switch(*(pointer+2))
	{
		case 0x84 : /*Read request from node to central*/ read_from_central(pointer,len); break;     
		case 0x85 : /*Node-read data write to central*/ write_to_central(pointer,len); break; 
		case 0x86 : /*Acknowledgment is sent from the node to the central */ break;      
		default   : break;
	}
	
}

void write_to_central(unsigned char* pointer,unsigned char len)
{
	//The data read from the node's sensors  is sent to the central node
	
	unsigned char receivednodeidmsb   = *(pointer);        //The received node-Id MSB
	unsigned char receivednodeidlsb   = *(pointer+1);      //The received node-Id LSB 
	unsigned char length              = *(pointer+3);      //The length of the received data
	unsigned char base_address        = *(pointer+4);      //The initial_address from where the data is read
	//It is assumed that the user processes this data according to his will and wish
	//Now send an Acknowledgment to the node 
	UART_transmit(receivednodeidmsb);
	UART_transmit(receivednodeidlsb);
	UART_transmit(0x82);          //ack code for the node 
}

void read_from_central(unsigned char* pointer,unsigned char length_byte)
{
  //The corresponding node has asked the central  it's (node's) configuration 
  //Central must send it's (corresponding node's) configuration and the corresponding node should send an acknowledgment 
  	unsigned char receivednodeidmsb   = *(pointer);        //The received node-Id MSB
  	unsigned char receivednodeidlsb   = *(pointer+1);      //The received node-Id LSB
  	unsigned char length              = *(pointer+3);      //The length of the received data
  	unsigned char base_address        = *(pointer+4);      //The initial_address from where the data is read
  //It is assumed that the user automatically writes the current configuration in the pointer data field starting from ptr+5 till ptr+length+4
  //Here base address and the length byte are the corresponding node's ACTUATOR'S base address and number of actuators to be written from that address
  //SENSORS CAN'T BE WRITTEN , the user must put the data to write in the pointer variable , finally transmit the CRC
  UART_transmit(receivednodeidmsb);
  UART_transmit(receivednodeidlsb);
  UART_transmit(0x81);            //The function code to write the actuator data to the node's actuators
  UART_transmit(length);
  UART_transmit(base_address);
  unsigned char i = 0x00;
  while(i<length_byte)
  {
	  //i  goes from 0 to length_byte-1 , so this ptr goes from 5 to length_byte+4 
	  UART_transmit(*(pointer+5+i));
	  i++;
  }
  //finally transmit the CRC
  UART_transmit(crc_calc(pointer,(length+4))); 
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
	key = key<<1;                      //Only the seven least significant bits to be used 
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
	}
	return data_high;
	/*For calculating 48 bit CRC , append 7 zeros to the 48 bits
	Then , keep appending the successive data till the last bit is exhausted
	Then use the zeros*/
}