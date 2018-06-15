/*
 * RFM_TEST_RAW_ATMEGA32.c
 *
 * Created: 12-06-2018 14:15:49
 *  Author: Bhushan Deo
 */ 

#define F_CPU 11059200
#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>
void SPI_init();
void UART_init();
void UART_transmit(unsigned char);
void SPI_write(unsigned char ,unsigned char );
unsigned char SPI_read(unsigned char );
void common_config();          //common configuration registers 
unsigned char receiver_config();        //a function(to be implemented in the future) which enables the receiver 
void transmitter_config();      //a function(to be implemented in the future) which enables the transmitter
void IRQ_DIO_config();         //IRQ and DIO config (to be implemented in the future)
void go_to_standby();         //a function(to be implemented in the future) which enables the standby mode 
void packet_engine_config();  //configure the packet engine registers (to be implemented in the future)
void test_config();           //configure the test registers (to be implemented in the future)

int main(void)
{
	UART_init();
	SPI_init();
	UART_transmit('B');
	UART_transmit('H');
	UART_transmit('U');
	UART_transmit('S');
	UART_transmit('H');
	UART_transmit('A');
	UART_transmit('N');
	unsigned char read_data;
    
    //TODO:: Please write your application code 
	//Here we configure each of the registers and check whether the configuration is correct 
	
	
	_delay_ms(3000);
	
	SPI_write(0x01,0x04);  //opmode : sequencer on , listen off , standby mode 
	read_data=SPI_read(0x01);
	UART_transmit(read_data);
	_delay_ms(1000);
	
	SPI_write(0x02,0x00);  //data-modulation : packet mode(we'll do fixed length) , FSK and no shaping 
	read_data=SPI_read(0x02);
	UART_transmit(read_data);
	_delay_ms(1000);
	
	SPI_write(0x03,0x1a);  //bit-rateMSB :we'll operate in the default 4.8kb/s
	read_data=SPI_read(0x03);
	UART_transmit(read_data);
	_delay_ms(1000);
	
	SPI_write(0x04,0x0b);  //bit-rateLSB :we'll operate in the default 4.8kb/s
	read_data=SPI_read(0x04);
	UART_transmit(read_data);
	_delay_ms(1000);
	
	SPI_write(0x05,0x00);  //fdevMSB :we'll operate at standard 5kHz freq. deviation 
	read_data=SPI_read(0x05);
	UART_transmit(read_data);
	_delay_ms(1000);
	
	SPI_write(0x06,0x52);  //fdevLSB :we'll operate at standard 5kHz freq. deviation 
	read_data=SPI_read(0x06);
	UART_transmit(read_data);
	_delay_ms(1000);
	
	//D92004 : the value of the RegFrf register 
	
	SPI_write(0x07,0xD9);  
	read_data=SPI_read(0x07);
	UART_transmit(read_data);
	_delay_ms(1000);
	
	SPI_write(0x08,0x20);
	read_data=SPI_read(0x08);
	UART_transmit(read_data);
	_delay_ms(1000);
	
	SPI_write(0x09,0x04);
	read_data=SPI_read(0x09);
	UART_transmit(read_data);
	_delay_ms(1000);
	
	SPI_write(0x11,0x9F);        //Transmitter RegPALevel : PA0 , and +13dBm 
	read_data=SPI_read(0x11);
	UART_transmit(read_data);
	_delay_ms(1000);
	
	SPI_write(0x12,0x09);       //The Tx Ramp-Up Time set to default 40 uS
	read_data=SPI_read(0x12);
	UART_transmit(read_data);
	_delay_ms(1000);
    
	
	SPI_write(0x13,0x1A);       //Ocp : output current protection : 95mA default 
	read_data=SPI_read(0x13);
	UART_transmit(read_data);
	_delay_ms(1000);
	
	SPI_write(0x18,0x04);       //Lna gain is set to 50 ohms and AGC is enabled 
	read_data=SPI_read(0x18);
	UART_transmit(read_data);
	_delay_ms(1000);
	
	
	SPI_write(0x19,0x55);       //Bandwidth : 10kHz , DC Cancellation : 4%
	read_data=SPI_read(0x19);
	UART_transmit(read_data);
	_delay_ms(1000);
	
	//rssi_config and rssi_value are used while in receiver mode 
	//DIO pin mappings to map the interrupts to the DIO pins 
	
	SPI_write(0x26,0x03);       //Clock-Out : Off to save power
	read_data=SPI_read(0x26);
	UART_transmit(read_data);
	_delay_ms(1000);
	
	//clear the regIRQ flags while leaving the Receive modes (just like we clear the interrupts)
	
	//To ask about the Rssi threshold , default is 114dBm 
	
	SPI_write(0x29,0xE4);       //Rssi set to -114dBm as default 
	read_data=SPI_read(0x29);
	UART_transmit(read_data);
	_delay_ms(1000);
	
	//we'll disable TimeoutRxStart and TimeOutRSSIThreshold for the time being (refer the datasheet table for additional details )
	
	//size of the preamble depends upon the type of system we're using 
	//That is fixed payload-length , node address , sync word(s) if used 
	
	SPI_write(0x2e,0x40);       //Only FiFo fill condition is enabled , sync address decoding is disabled for the time being 
	read_data=SPI_read(0x2e);
	UART_transmit(read_data);
	_delay_ms(1000);
	
	//for bit synchronization only syncregvalue1 has to be entered 0xAA and 0x55 , here we neglect the bit synchronizer 
	
	SPI_write(0x37,0x02);       //Fixed length , no encoding , CRC off , only node-address is matched
	read_data=SPI_read(0x37);
	UART_transmit(read_data);
	_delay_ms(1000);
	
	//Now the payload-length to specify using Reg-Payload-length
	SPI_write(0x38,0x01);       //Payload length is fixed to a byte for now 
	read_data=SPI_read(0x38);
	UART_transmit(read_data);
	_delay_ms(1000);
	
	
	SPI_write(0x39,0x05);       //Node-Address used in address filtering 
	read_data=SPI_read(0x39);
	UART_transmit(read_data);
	_delay_ms(1000);
	
	SPI_write(0x3C,0x80);       //Node-Address used in address filtering
	read_data=SPI_read(0x3C);
	UART_transmit(read_data);
	_delay_ms(1000);
	
	SPI_write(0x3D,0x02);       //enable auto-restart after payloadready is generated and FIFO is read by the controller 
	read_data=SPI_read(0x3D);
	UART_transmit(read_data);
	_delay_ms(1000);
	
	//test registers are configured as default 
}
void UART_init()
{
	UCSRC=0;
	UBRRH=0x00;
	UBRRL=0x47;			//storing lower bits here.
	UCSRB=1<<TXEN;		//enable receiver and transmitter
	UCSRC=1<<URSEL|1<<UCSZ1|1<<UCSZ0;		//transmitting 8 bit data
}
void UART_transmit(unsigned char data)
{
	while(!(UCSRA&(1<<UDRE)));		//loop will run till the buffer is empty
	UDR=data;		//then complete data is copied to UDR
}
void SPI_init()
{
	//initialize the SPI communication 
	//with the slave select to be output and configured low then high 
	DDRB|=(1<<PB4)|(1<<PB5)|(1<<PB7);
	PORTB|=(1<<PB4);
	SPCR=(1<<SPE)|(1<<MSTR);
	SPSR=(1<<SPI2X);
}
void SPI_write(unsigned char address,unsigned char data)
{
	PORTB&=~(1<<PB4);
	//write the arguments 
	SPDR=address|0x80;		
	while(!(SPSR&(1<<SPIF)));		// program will wait till complete data is transfered
	unsigned char garbage = SPDR;
	SPDR=data;		
	while(!(SPSR&(1<<SPIF)));		// program will wait till complete data is transfered
	garbage = SPDR;
	PORTB|=(1<<PB4);
}
unsigned char SPI_read(unsigned char address)
{
	PORTB&=~(1<<PB4);
	SPDR=address&0x7F;		
	while(!(SPSR&(1<<SPIF)));		// program will wait till complete data is transfered
	unsigned char garbage = SPDR;
	SPDR=0x00;		
	while(!(SPSR&(1<<SPIF)));		// program will wait till complete data is transfered
	unsigned char new = SPDR;
	PORTB|=(1<<PB4);
	return new;
	
}