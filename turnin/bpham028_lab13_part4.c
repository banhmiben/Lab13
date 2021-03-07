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
#include <scheduler.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

unsigned char pattern = 0x80;
unsigned char row = 0xFE;

void Set_A2D_Pin(unsigned char pinNum) {
	ADMUX = (pinNum <= 0x07) ? pinNum : ADMUX;
	static unsigned char i = 0;
	for (i = 0; i < 15; i++) { asm("nop"); }
}

void ADC_init() {
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
	//ADEN: setting this bit enables analog-to-digital conversion
	//ADSC: setting this bit starts the first conversion.
	//ADATE: setting this bit enables auto-triggering. Since we are in
			// Free Running Mode, a new conversion will trigger whenever the previous conversion completes
}

enum leftright_states {start, init, wait, left, right};
int LRTick(int state) {

	Set_A2D_Pin(0x00);
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
			pattern = 0x80;
			break;
		case(wait):
			break;
		case(left):
			if (pattern < 0x80) {
				pattern = pattern << 1;
			} break;
		case(right):
			if (pattern > 0x01) {
				pattern = pattern >> 1;
			} break;
		default:
			break;
	}
	PORTC = pattern;
	PORTD = row;
	return state;
}

enum updown_states {start1, init1, wait1, up, down};
int UDTick(int state) {

	Set_A2D_Pin(0x01);
	switch(state) {
		case(start1):
			state = init1;
			break;
		case(init1):
			state = wait1;
			break;
		case(wait1):
			if (ADC < 542 - 100) {
				state = down;
			} else if (ADC > 542 + 100) {
				state = up;
			} else {
				state = wait1;
			} break;
		case(up):
			state = wait1;
			break;
		case(right):
			state = wait1;
			break;
		default:
			state = start1;
			break;
	}
	switch(state) {
		case(start1):
			break;
		case(init1):
			row = 0xFE;
			break;
		case(wait1):
			break;
		case(up):
			if (row < 0xFE) {
				row = (row >> 1) | 0x80;
			} break;
		case(down):
			if (row > 0xEF) {
				row = (row << 1) | 0x01;
			} break;
		default:
			break;
	}
	PORTC = pattern;
	PORTD = row;
	return state;
}

enum ADC_states {start2, LR, UD};
int ADCSetTick(int state) {

	switch(state) {
		case(start2):
			state = LR;
			break;
		case(LR):
			state = UD;
			break;
		case(UD):
			state = LR;
			break;
		default:
			state = start2;
			break;
	}
	switch(state) {
		case(start2):
			break;
		case(LR):
			Set_A2D_Pin(0x00);
			break;
		case(UD):
			Set_A2D_Pin(0x02);
			break;
		default:
			break;
	}
	return state;
}

int main(void) {
	DDRA = 0x00; PORTA = 0xFF;
//	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	ADC_init();

	static task task1, task2, task3;
	task *tasks[] = {&task1, &task2, &task3};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

	const char starter = -1;
	//SpeedSM
	task1.state = starter;
	task1.period = 75;
	task1.elapsedTime = task1.period;
	task1.TickFct = &LRTick;
	//ShiftSM
	task2.state = starter;
	task2.period = 75;
	task2.elapsedTime = task2.period;
	task2.TickFct = &UDTick;
	//OutputSM
	task3.state = starter;
	task3.period = 1;
	task3.elapsedTime = task3.period;
	task3.TickFct = &ADCSetTick;

	TimerSet(1);
	TimerOn();

	unsigned short i; //scheduler loop iterator

	while(1) {
		for (i = 0; i < numTasks; i++) {
			if (tasks[i] -> elapsedTime == tasks[i] -> period) {
				tasks[i] -> state = tasks[i] -> TickFct(tasks[i] -> state);
				tasks[i] -> elapsedTime = 0;
			}
			tasks[i] -> elapsedTime += 1;
		}
		while(!TimerFlag);
		TimerFlag = 0;
	}
	return 0;

}
