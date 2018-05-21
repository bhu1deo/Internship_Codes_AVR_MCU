/*
 * ADC_header.c
 *
 * Created: 21-05-2018 15:37:41
 *  Author: Bhushan
 */ 

#define F_CPU 1000000
#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>
#include"ADC_header.h"
unsigned char array[3];
unsigned char start_ADC_conversion(unsigned char vref, unsigned char pin)
{
	unsigned char* array_new;
	unsigned char temp;
	//Set the inputs according to the Vref and the pin (input pin for the ADC)
	/*vref=1 corresponds to 5V connected to the AVcc pin*/
    /*vref=0 corresponds to the internal 2.56V with external cap at AVref pin */
	if(vref)   
	{
		//5V as the Vref
		//Check which pin the input is connected to 
		array_new=dec_to_binary(pin);        //To adjust the MUX values 
		//DDRB=0xFF;
		//PORTB=(array_new[0]<<0)|(array_new[1]<<1)|(array_new[2]<<2); //Just to check whether the returned value is correct or not 
		//DDRC=0xFF; //the 8 bits of the ADC conversion to be displayed on the C port ADCL bits
		//DDRD=0xFF;   //ADCH 2 bits
		ADMUX=(1<<REFS0)|(1<<ADLAR)|(array_new[0]<<MUX0)|(array_new[1]<<MUX1)|(array_new[2]<<MUX2); //put your source on the ADC0 bit so no need to adjust the MUX bits , 5V is the top of the ADC
		ADCSRA=(1<<ADEN)|(1<<ADPS1)|(1<<ADSC);  //start conversion , enable the ADC ,  clock freq : fclk/4
		while(!(ADCSRA&(1<<ADIF)));  //wait till the conversion is over
        temp=ADCH;
		ADCSRA=(1<<ADIF);       //clear the ADC flag manually by writing one to it
	}
	else 
	{
		//2.56V as the Vref
		//Check which pin the input is connected to
		array_new=dec_to_binary(pin);        //To adjust the MUX values
		//DDRB=0xFF;
		//PORTB=(array_new[0]<<0)|(array_new[1]<<1)|(array_new[2]<<2); //Just to check whether the returned value is correct or not
		//DDRC=0xFF; //the 8 bits of the ADC conversion to be displayed on the C port ADCL bits
		//DDRD=0xFF;   //ADCH 2 bits
		ADMUX=(1<<REFS1)|(1<<REFS0)|(1<<ADLAR)|(array_new[0]<<MUX0)|(array_new[1]<<MUX1)|(array_new[2]<<MUX2); //put your source on the ADC0 bit so no need to adjust the MUX bits , 5V is the top of the ADC
		ADCSRA=(1<<ADEN)|(1<<ADPS1)|(1<<ADSC);  //start conversion , enable the ADC ,  clock freq : fclk/4
		while(!(ADCSRA&(1<<ADIF)));  //wait till the conversion is over
		temp=ADCH;
		ADCSRA=(1<<ADIF);       //clear the ADC flag manually by writing one to it
	}
	/*Left shifted result with 15mV of error*/
	return temp;
}
unsigned char* dec_to_binary(unsigned char pin)
{
	array[0]=0;
	array[1]=0;
	array[2]=0;
	if(pin==0)
	{
		return array;
	}
	else
	{
	unsigned char i=0;
	while(pin>1)
	{
		array[i]=(unsigned char )(pin%2);
		pin=(unsigned char)(pin/2);
		i++;
	}
	array[i]=1;
	}
	return array;
}