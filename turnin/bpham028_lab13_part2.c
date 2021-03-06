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
			//PORTC = pattern1;
			//PORTD = row1;
			break;
		case(left):
			if (pattern1 == 0x80) {
				pattern1 = 0x01;
			} else {
				pattern1 = pattern1 << 1;
			} break;
		case(right):
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
	TimerSet(100);
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
