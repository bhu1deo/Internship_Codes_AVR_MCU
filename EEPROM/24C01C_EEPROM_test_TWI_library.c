/*
 * 24C01C_EEPROM_test_TWI_library.c
 *
 * Created: 31-05-2018 14:35:27
 *  Author: Bhushan Deo copyright : Bhushan Deo , all disputes to be resolved at Bombay High Court 
 */ 
//This is a test code for the 24C01C eeprom module using the standard TWI library created by Bhushan Deo (see github.com/bhu1deo)
//We check the start condition , address transfer , data transfer by glowing LEDs connected on PORTD , the library functions are manipulated according 
// to our convenience 
#define F_CPU 11059200
#include<avr/io.h>
#include<util/delay.h>
#include<avr/interrupt.h>
//#include"TWI_header.h"
//Here we communicate through the SPI @100kHz 
//So we load the TWBR with decimal value of 47 
//So the SCL freq becomes = 11059200/(16+2(47)(1))=11059200/110=100kHz
//We will either operate in master transmit or master receive modes only 
//While writing to the EEPROM first transmit the device address and then the memory address 
int main(void)
{
	DDRD=0xFF;
	//First transmit the start condition 
	//Then the SLA+W , so as to select that particular slave
	//Then the memory register address (to be written as data???) 
	//Then the data byte 
	//Then a stop condition 
	unsigned char start_flag=0;     
	unsigned char device_address_flag=0;
	unsigned char memory_address_flag=0;
	unsigned char data_flag=0;
	unsigned char written =0;
	unsigned char device_address=0b01010000; // the device address
	unsigned char memory_address=0x00;
	unsigned char data=0xAF;
	unsigned char address_read=0;
	//We will begin the master transmitter mode
	TWBR=0x2F;
	TWCR=(1<<TWINT)|(1<<TWSTA)|(1<<TWEN);         //clear the interrupt flag to avoid running of the ISR , set the start bit to transmit the start bit
	TWSR=0x00;                                    // enable the start condition , clear the status register 
    while(!written)
    {
        //Set the written variable to 1 when the writing process is done to avoid re-running of the loop
		if((TWSR==0x08)&&(start_flag==0))  //Start bit has been received now transmit the device address 
		{
			//device_address  is a 7 bit variable not a byte 
			start_flag=1;
			TWDR=(device_address)<<1;  // To transmit the address and a zero appended at the lsb in order to enable write mode
			TWCR=(1<<TWINT)|(1<<TWEN);  //clear the flag to transmit
			PORTD=0x01;   //first led displays the start bit
			_delay_ms(10);
		}
		else if((TWSR==0x18)&&(device_address_flag==0))  //device_address has been received  now the memory address
		{
			//device_addr=0;
			device_address_flag=1;
			TWDR=memory_address;  //send the memory_address
			TWCR=(1<<TWINT)|(1<<TWEN);    //clear the flag to transmit
			PORTD=0x10;     //  displays the matching of the address
			_delay_ms(10);
		}
		else if((TWSR==0x28)&&(memory_address_flag==0))  //Memory_address  has been received only then transmit the data
		{
			memory_address_flag=1;
			TWDR=data;
			//transmit the stop bit
			TWCR=(1<<TWINT)|(1<<TWEN);
			PORTD=0x04;       //third led displays the matching of the data
			_delay_ms(10);
			
		}
		else if((TWSR==0x28)&&(data_flag==0))  //Data  has been received only then transmit the stop flag 
		{
			data_flag=1;
			//TWDR=data;
			//transmit the stop bit
			TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
			PORTD=0xFF;       //third led displays the matching of the data
			_delay_ms(10);
			written=1;
		}
    }
	_delay_ms(20);
	unsigned char start_flag_new=0;  //for the read operation 
	unsigned char received = 0;
	start_flag=0;
	device_address_flag=0;
	unsigned char device_address_flag_new=0;
	memory_address_flag=0;
	data_flag=0;
	//Read operation :  Just send the start , SLA+W , word address , stop as a part of write operation 
	//Then again generate a start , SLA+R , stop after receiving the data but do not acknowledge or else data at the next memory location would be sent 
	//Now assuming that the code runs correctly and data is written to a memory location , we will start the process of reading the data
	TWBR=0x2F;
	TWCR=(1<<TWINT)|(1<<TWSTA)|(1<<TWEN);         //clear the interrupt flag to avoid running of the ISR , set the start bit to transmit the start bit
	TWSR=0x00;                                    // enable the start condition , clear the status register
	device_address=0b01010000;
	while(!address_read)
	{
		//First transmit the address to be read (memory_address) as a part of the write operation 
		//Then initialize another read operation thereby getting the data 
		if((TWSR==0x08)&&(start_flag==0))  //Start bit has been received now transmit the device address
		{
			//device_address is a 7 bit variable not a byte
			start_flag=1;
			TWDR=(device_address)<<1;  // To transmit the address and a zero appended at the lsb in order to enable write mode
			TWCR=(1<<TWINT)|(1<<TWEN);  //clear the flag to transmit
			PORTD=0x13;   
			_delay_ms(10);
		}
		else if((TWSR==0x18)&&(device_address_flag==0))  //device_address has been received  now the memory address
		{
			//device_addr=0;
			device_address_flag=1;
			TWDR=memory_address;  //send the memory_address
			TWCR=(1<<TWINT)|(1<<TWEN);    //clear the flag to transmit
			PORTD=0x14;     //  displays the matching of the address
			_delay_ms(10);
		}
		else if((TWSR==0x28)&&(memory_address_flag==0))  //Memory_address  has been received now again transmit the start flag to read the data 
		{
			memory_address_flag=1;
			//TWDR=data;
			//transmit the stop bit
			TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWSTA);
			PORTD=0x15;      
			_delay_ms(10);	
		}
		else if((TWSR==0x10))  //Repeated Start bit has been received now transmit the device address
		{
			//device_address is a 7 bit variable not a byte
			start_flag_new=1;
			TWDR=((device_address)<<1)|0x01;  // To transmit the address and a one appended at the lsb in order to enable read mode
			TWCR=(1<<TWINT)|(1<<TWEN);  //clear the flag to transmit
			PORTD=0x16;   
			_delay_ms(10);
		}
		else if((TWSR==0x40)&&(device_address_flag==1)&&(device_address_flag_new==0))  //device_address has been received  read the data byte 
		{
			//device_addr=0;
			device_address_flag_new=1;
			//sTWDR=memory_address;  //send the memory_address
			TWCR=(1<<TWINT)|(1<<TWEN);    //clear the flag to transmit
			PORTD=0x17;     //  displays the matching of the address
			_delay_ms(10);
		}
		else if((TWSR==0x58)&&(received==0))
		{
			DDRB=0xFF;
			received = 1;
			PORTD=0x18;
			//The data has been received but not acknowledged to stop the reading process
			PORTB=TWDR;
			TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWSTO);       //Again clear the TWINT bit so as to enable for future transmissions and transmit the stop bit
			_delay_ms(10);
			address_read=1;
		}
		
	}
}