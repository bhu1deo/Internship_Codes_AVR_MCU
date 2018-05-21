/*
 * USART_header.c
 *
 * Created: 14-05-2018 13:00:37
 *  Author: Bhushan
 */ 
/*Control the baud rate , frame select , synchronous or asynchronous mode , master or slave in synchronous modes , */
/*Data would be transmitted in standard 8 bit mode.*/
/*Parity and number of stop bits could be selected.*/
#ifndef USART_header_h
#define USART_header_h
//Header file for the USART communication 
/*Be sure to disable the interrupts of the concerned flags to avoid ISR to get continuously executed */
#define F_CPU 8000000               //8MHz is good as it reduces the error in baud rate 
#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>
#include<stdbool.h>
unsigned int baudrate ;            //Required perfect baudrate (standard) must be passed as an argument
unsigned char mode;                // synchronous or asynchronous
unsigned char  stop;                //Number of stop bits in the communication process 
unsigned char parity0;             //These two bits indicate the parity combinations of the data communication 
 unsigned char parity1;
 unsigned char tr;                  //Initialize as transmitter or receiver 
unsigned char c;                   //The byte of data to be transmitted to the receiver 
//The functions used here are as follows:
void initialize_USART(unsigned int baudrate,unsigned char mode,unsigned char stop,unsigned char parity0,unsigned char parity1,unsigned char tr);
void send_data(unsigned char c);        //After initializations have been done , just send and receive data
unsigned char receive_data();
#endif
