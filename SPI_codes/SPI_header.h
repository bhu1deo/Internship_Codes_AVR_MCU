/*
 * SPI_header.c
 *
 * Created: 15-05-2018 12:54:04
 *  Author: Bhushan
 */ 
#ifndef _SPI_header_h
#define _SPI_header_h
#define F_CPU 1000000      //1MHz of clock frequency
#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>
#include<stdbool.h>
unsigned char port ;
unsigned char pin;
unsigned char ms ;
unsigned char data;
void initialize(unsigned char ms ,unsigned char port , unsigned char pin );
unsigned char send_read_data(unsigned char data,unsigned char ms );
#endif
