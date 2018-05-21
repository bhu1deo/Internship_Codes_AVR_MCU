#define F_CPU 1000000
#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>
unsigned char vref;
unsigned char pin;
unsigned char start_ADC_conversion(unsigned char vref, unsigned char pin);
unsigned char* dec_to_binary(unsigned char pin);