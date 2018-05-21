/*
 * TWI_header.c
 *
 * Created: 21-05-2018 09:22:22
 *  Author: Bhushan
 */ 
/*TWI library header file for the atmega32 microcontroller*/
#define F_CPU 1000000
#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>
/*A function to initiate the TWI communication */
/*MT SR or MR ST modes*/
/*Address and data to be taken from the user */
unsigned char TWIcomm(unsigned char mode,unsigned char address,unsigned char data,unsigned char device);
//The user must pass the mode he wants to use , if MT-SR is to be used then char M needs to be transmitted else char S needs to be transmitted
//The address of the slave and the data to be transmitted also has to be given , we are transmitting in a single start-stop condition
//Multiple start-stop condition is not entertained in this library 
//Device variable is used to check whether the device to be programmed is a master or a slave 
//Bit rate register is kept at 0 , if required please edit in the source file 
//This function returns the data received in TWIcommunication 
//If acting as a transmitter(MT or ST) , then the function returns a 0 
