/*
 * DS1307_RTC.c
 * Done for the atmega32
 * Created: 13-06-2018 12:09:18
 *  Author: Bhushan
 */ 

/*-----------------------Initializing header and macros--------------------------------------------*/

#define F_CPU 11059200   //simulating with the exact external crystal oscillator 
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>
uint8_t device_address = 0b01101000; //unfortunately same device address for all the RTCs , so use only one RTC
uint8_t memory_address;              //configuring the registers of the RTC
uint8_t start_ack=0x08;              //the start acknowledge
uint8_t device_ack=0x18;             //the device_address acknowledge
uint8_t mem_data_ack=0x28;
void RTC_write(uint8_t ,uint8_t ,uint8_t );
uint8_t RTC_read(uint8_t ,uint8_t );
void TWI_send(uint8_t ,bool );

/*-----------------------Comments regarding the code --------------------------------------------*/

//So the SCL freq becomes = 11059200/(16+2(47)(1))=11059200/110=100kHz


/*-----------------------Main--------------------------------------------*/

int main(void)
{
	
	
	
	DDRA=0xFF; //display the RTC data on this PORT
	while(1)
	{
		//hrs,mins,secs were read and displayed on PORTA
		//RTC_write(device_address,memory_address,datay);
		PORTA=RTC_read(device_address,0x02);
		_delay_ms(500);
		PORTA=RTC_read(device_address,0x01);
		_delay_ms(500);
		PORTA=RTC_read(device_address,0x00);
		_delay_ms(500);
		
	}
	
    while(1)
    {
        //TODO:: Please write your application code 
    }
}

/*-----------------------RTC_write--------------------------------------------*/
 
void RTC_write(uint8_t device_address,uint8_t memory_address,uint8_t data)
{
	//We will begin the master transmitter mode
	TWBR=0x2F;
	TWCR=(1<<TWINT)|(1<<TWSTA)|(1<<TWEN);         //clear the interrupt flag to avoid running of the ISR , set the start bit to transmit the start bit
	TWSR=0x00;                                    // enable the start condition , clear the status register
	//start condition doesn't need any memory or device address directly transmit during initializations
	//write to the individual registers of the RTC
	//first send the device_address with the start_ack 
	uint8_t start_flag=0;     //for the start condition 
	uint8_t device_address_flag=0;  //for the device_address
	uint8_t memory_address_flag=0;  //for the memory address 
	uint8_t data_flag=0;            //for the data
	uint8_t written =0;             //a variable to complete the writing process
	while(!written)
	{
		//Set the written variable to 1 when the writing process is done to avoid re-running of the loop
		if((TWSR==0x08)&&(start_flag==0))  //Start bit has been received now transmit the device address
		{
			//device_address  is a 7 bit variable not a byte
			start_flag=1;
			TWDR=(device_address)<<1;  // To transmit the address and a zero appended at the lsb in order to enable write mode
			TWCR=(1<<TWINT)|(1<<TWEN);  //clear the flag to transmit
			_delay_ms(1);
		}
		else if((TWSR==0x18)&&(device_address_flag==0))  //device_address has been received  now the memory address
		{
			//device_addr=0;
			device_address_flag=1;
			TWDR=memory_address;  //send the memory_address
			TWCR=(1<<TWINT)|(1<<TWEN);    //clear the flag to transmit
			//PORTD=0x10;     //displays the matching of the address
			_delay_ms(1);
		}
		else if((TWSR==0x28)&&(memory_address_flag==0))  //Memory_address  has been received only then transmit the data
		{
			memory_address_flag=1;
			TWDR=data;
			//transmit the stop bit
			TWCR=(1<<TWINT)|(1<<TWEN);
			//PORTD=0x04;       //third led displays the matching of the data
			_delay_ms(1);
			
		}
		else if((TWSR==0x28)&&(data_flag==0))  //Data  has been received only then transmit the stop flag
		{
			data_flag=1;
			//TWDR=data;
			//transmit the stop bit
			TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
			//PORTD=0xFF;       //third led displays the matching of the data
			_delay_ms(1);
			written=1;
		}
	}
}

/*-----------------------RTC_read--------------------------------------------*/

uint8_t RTC_read(uint8_t device_address,uint8_t memory_address)
{

	uint8_t received = 0;
	uint8_t start_flag=0;
	uint8_t device_address_flag=0;
	uint8_t device_address_flag_new=0;
	uint8_t memory_address_flag=0;
	//uint8_t address_read=0;
	//Read operation :  Just send the start , SLA+W , word address , stop as a part of write operation
	//Then again generate a start , SLA+R , stop after receiving the data but do not acknowledge or else data at the next memory location would be sent
	//Now assuming that the code runs correctly and data is written to a memory location , we will start the process of reading the data
	//read from the registers of the RTC
	uint8_t read_data;    //data to be returned by the RTC's memory address 
	TWBR=0x2F;
	TWCR=(1<<TWINT)|(1<<TWSTA)|(1<<TWEN);         //clear the interrupt flag to avoid running of the ISR , set the start bit to transmit the start bit
	TWSR=0x00;                                    // enable the start condition , clear the status register
	while(!received)
	{
		//First transmit the address to be read (memory_address) as a part of the write operation
		//Then initialize another read operation thereby getting the data
		if((TWSR==0x08)&&(start_flag==0))  //Start bit has been received now transmit the device address
		{
			//device_address is a 7 bit variable not a byte
			start_flag=1;
			TWDR=(device_address)<<1;  // To transmit the address and a zero appended at the lsb in order to enable write mode
			TWCR=(1<<TWINT)|(1<<TWEN);  //clear the flag to transmit
			//PORTD=0x13;
			_delay_ms(1);
		}
		else if((TWSR==0x18)&&(device_address_flag==0))  //device_address has been received  now the memory address
		{
			//device_addr=0;
			device_address_flag=1;
			TWDR=memory_address;  //send the memory_address
			TWCR=(1<<TWINT)|(1<<TWEN);    //clear the flag to transmit
			//PORTD=0x14;     //  displays the matching of the address
			_delay_ms(1);
		}
		else if((TWSR==0x28)&&(memory_address_flag==0))  //Memory_address  has been received now again transmit the start flag to read the data
		{
			memory_address_flag=1;
			//TWDR=data;
			//transmit the stop bit
			TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWSTA);
			//PORTD=0x15;
			_delay_ms(1);
		}
		else if((TWSR==0x10))  //Repeated Start bit has been received now transmit the device address
		{
			//device_address is a 7 bit variable not a byte
			TWDR=((device_address)<<1)|0x01;  // To transmit the address and a one appended at the lsb in order to enable read mode
			TWCR=(1<<TWINT)|(1<<TWEN);  //clear the flag to transmit
			//PORTD=0x16;
			_delay_ms(1);
		}
		else if((TWSR==0x40)&&(device_address_flag==1)&&(device_address_flag_new==0))  //device_address has been received  read the data byte
		{
			//device_addr=0;
			device_address_flag_new=1;
			//sTWDR=memory_address;  //send the memory_address
			TWCR=(1<<TWINT)|(1<<TWEN);    //clear the flag to transmit
			//PORTD=0x17;     //  displays the matching of the address
			_delay_ms(1);
		}
		else if((TWSR==0x58)&&(received==0))
		{
			DDRB=0xFF;
			received = 1;
			//PORTD=0x18;
			//The data has been received but not acknowledged to stop the reading process
			read_data=TWDR;
			TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWSTO);       //Again clear the TWINT bit so as to enable for future transmissions and transmit the stop bit
			_delay_ms(1);
		}
		
	}
	return read_data;
}





