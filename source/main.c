/*	Author: lab
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #  Exercise #
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */

#include <avr/io.h>
#include <timer.h>
//#include <scheduler.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

#define shiftC 0x01 //pattern
#define shiftD 0x02 //row

unsigned char pattern1 = 0x00;
unsigned char row1 = 0x00;

void ADC_init() {
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
	//ADEN: setting this bit enables analog-to-digital conversion
	//ADSC: setting this bit starts the first conversion.
	//ADATE: setting this bit enables auto-triggering. Since we are in
			// Free Running Mode, a new conversion will trigger whenever the previous conversion completes
}

void transmit_data(unsigned char data, unsigned char shiftNum) {
	int i;
	for (i = 0; i < 8; ++i) {
		//Sets SRCLR to 1 allowing data to be set
		//Also clears SRCLK in preparation of sending data (shift1 SRCLR = 0x08 shift2 SRCLR = 0x20)
		if (shiftNum == shiftC) {
			PORTC = 0x08;
		} else if (shiftNum == shiftD) {
			PORTC = 0x20;
		}
		//set SER = next bit of data to be sent.
		PORTC |= ((data >> i) & 0x01);
		//set SRCLK = 1. Rising edge shifts next bit of data into the shift register
		PORTC |= 0x02;
	}
	//set RCLK = 1. Rising edge copies data from "Shift" register to "Storage" register
	if (shiftNum == shiftC) {
		PORTC |= 0x04;
	} else if (shiftNum == shiftD) {
		PORTC |= 0x10;
	}
	//clears all lines in preparation of a new transmission
	PORTC = 0x00;
}

enum states {start, init, wait, left, right} state;
void Tick() {

	row1 = 0xFE;
	switch(state) {
		case(start):
			state = init;
			break;
		case(init):
			state = wait;
			break;
		case(wait):
			if (ADC < 542 - 100) {
				state = left;
			} else if (ADC > 542 + 100) {
				state = right;
			} else {
				state = wait;
			} break;
		case(left):
			state = wait;
			break;
		case(right):
			state = wait;
			break;
		default:
			state = start;
			break;
	}
	switch(state) {
		case(start):
			break;
		case(init):
			pattern1 = 0x80;
			break;
		case(wait):
			PORTB = pattern1;
			PORTC = pattern1;
			PORTD = row1;
			break;
		case(left):
			PORTB = 0x02;
			if (pattern1 == 0x80) {
				pattern1 = 0x01;
			} else {
				pattern1 = pattern1 << 1;
			} break;
		case(right):
			PORTB = 0x04;
			if (pattern1 == 0x01) {
				pattern1 = 0x80;
			} else {
				pattern1 = pattern1 >> 1;
			} break;
		default:
			break;
	}
	PORTC = pattern1;
	PORTD = row1;
}


int main(void) {
//	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	TimerSet(10);
	TimerOn();
	ADC_init();
	state = start;
	while(1) {
		Tick();
		while(!TimerFlag){}
		TimerFlag = 0;
	}
	return 1;
}
