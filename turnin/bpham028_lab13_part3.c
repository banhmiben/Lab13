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

#define speed1000 100
#define speed500 50
#define speed250 25
#define speed100 10

unsigned char pattern1 = 0x00;
unsigned char row1 = 0x00;
unsigned char speed;
unsigned char cnt;

void ADC_init() {
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
	//ADEN: setting this bit enables analog-to-digital conversion
	//ADSC: setting this bit starts the first conversion.
	//ADATE: setting this bit enables auto-triggering. Since we are in
			// Free Running Mode, a new conversion will trigger whenever the previous conversion completes
}

enum Speed_States{start, init, wait, left, right};
int SpeedTick(int state) {
	
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
			PORTB = 0x01;
			speed = 0x00;
			break;
		case(wait):
			break;
		case(left):
			PORTB = 0x02;
			if ((ADC <= 400) && (ADC > 300)) {
				speed = 100;
				PORTB = 0x01;
			} else if ((ADC <= 300) && (ADC > 200)) {
				speed = 50;
				PORTB = 0x02;
			} else if ((ADC <= 200) && (ADC > 100)) {
				speed = 25;
				PORTB = 0x04;
			} else if (ADC <= 100) {
				speed  = 10;
				PORTB = 0x08;
			} break;
		case(right):
			PORTB = 0x04;
			if ((ADC >= 600) && ADC < 700) {
				speed = 100;
				PORTB = 0x01;
			} else if ((ADC >= 700) && (ADC < 800)) {
				speed = 50;
				PORTB = 0x02;
			} else if ((ADC >= 800) && (ADC < 900)) {
				speed = 25;
				PORTB = 0x04;
			} else if (ADC >= 900) {
				speed = 10;
				PORTB = 0x08;
			} break;
		default:
			break;
	}
	return state;
}
				
enum Shift_States {start1, init1, wait1, leftwait, rightwait, leftshift, rightshift};
int ShiftTick(int state) {

	//row1 = 0xFE;
	switch(state) {
		case(start):
			state = init1;
			break;
		case(init1):
			state = wait1;
			break;
		case(wait1):
			if (ADC < 542 - 100) {
				state = leftwait;
				cnt = 0;
			} else if (ADC > 542 + 100) {
				state = rightwait;
				cnt = 0;
			} else {
				state = wait1;
			} break;
		case(leftwait):
			if (cnt < speed) {
				state = leftwait;
			} else if (cnt >= speed) {
				state = leftshift;
			} break;
		case(rightwait):
			if (cnt < speed) {
				state = rightwait;
			} else if (cnt >= speed) {
				state = rightshift;
			} break;
		case(leftshift):
			state = wait1;
			break;
		case(rightshift):
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
			pattern1 = 0x80;
			break;
		case(wait1):
			cnt = 0;
			break;
		case(leftwait):
			++cnt;
			break;
		case(rightwait):
			++cnt;
			break;
		case(leftshift):
			if (pattern1 == 0x80) {
				pattern1 = 0x01;
			} else {
				pattern1 = pattern1 << 1;
			} break;
		case(rightshift):
			if (pattern1 == 0x01) {
				pattern1 = 0x80;
			} else {
				pattern1 = pattern1 >> 1;
			} break;
		default:
			break;
	}
	//PORTC = pattern1;
	//PORTD = row1;
	return state;
}

enum Output_States{start2, output};
int OutputTick(int state) {
	
	switch(state) {
		case(start2):
			state = output;
			break;
		case(output):
			state = output;
			break;
		default:
			state = start2;
			break;
	}
	switch(state) {
		case(start2):
			break;
		case(output):
			PORTC = pattern1;
			PORTD = 0xFE;
			break;
		default:
			break;
	}
	return state;
	
}


int main(void) {
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	ADC_init();
	
	static task task1, task2, task3;
	task *tasks[] = {&task1, &task2, &task3};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
	
	const char starter = -1;
	//SpeedSM
	task1.state = starter;
	task1.period = 10;
	task1.elapsedTime = task1.period;
	task1.TickFct = &SpeedTick;
	//ShiftSM
	task2.state = starter;
	task2.period = 10;
	task2.elapsedTime = task2.period;
	task2.TickFct = &ShiftTick;
	//OutputSM
	task3.state = starter;
	task3.period = 10;
	task3.elapsedTime = task3.period;
	task3.TickFct = &OutputTick;
	
	TimerSet(10);
	TimerOn();
	
	unsigned short i; //scheduler loop iterator
	
	while(1) {
		for (i = 0; i < numTasks; i++) {
			if (tasks[i] -> elapsedTime == tasks[i] -> period) {
				tasks[i] -> state = tasks[i] -> TickFct(tasks[i] -> state);
				tasks[i] -> elapsedTime = 0;
			} 
			tasks[i] -> elapsedTime += 10;
		}
		while(!TimerFlag);
		TimerFlag = 0;
	}
	return 0;
	
}
