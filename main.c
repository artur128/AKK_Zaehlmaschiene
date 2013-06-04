#include <avr/io.h>
#include <util/delay.h>
#include <util/twi.h> 
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>
#include <avr/eeprom.h>
#include "main.h"

#define BB 7
#define S 6
#define Q 4

volatile uint8_t state=0;
uint8_t zeig=8;
volatile uint8_t ui[9];
volatile uint16_t imw=14000;
volatile uint8_t  ir_max=0;
volatile uint8_t imw_can_set=0;

uint16_t geld[8];
uint16_t old_stop_geld[8];

void dus(unsigned int z){
	for(unsigned int i=0;i<z;i++) {_NOP(); _NOP(); _NOP();  _NOP(); _NOP(); _NOP(); _NOP(); _NOP(); _NOP();}
}


void beep(uint16_t hertz, unsigned int laenge){
	for(unsigned int i=0;i<laenge*10000;i+=100000/hertz){
		PORTB|=(1<<PB3);
		dus(500000/hertz);
		PORTB&=~(1<<PB3);
		dus(500000/hertz);
	}
}

void piep(const uint16_t ton[][2]){
	for(unsigned char k=0;ton[k][1]!=0;k++)
		beep(ton[k][0],ton[k][1]);
}
void init_sound(void){
	DDRB|=(1<<PB3);
}




void init_motor(void){
}


void set_motor_speed(uint8_t speed){
	DDRB|=(1<<PB4);
	_NOP();
	if (speed==0 || speed==7) { TCCR0A = 0; TCCR0B = 0;}
	if (speed==0) PORTB|=(1<<PB4);
	if (speed==7) PORTB&=~(1<<PB4);
	if (speed==0 || speed==7) return;
	TCCR0A = (1<<COM0B1) | (1<<WGM00) | (1<<WGM01);
	TCCR0B = (1<<CS01) | (1<<CS00) | (1<<WGM02);
	OCR0A = speed;
}

//    1
// 2     4
//    8
//128    16
//    64
//       32
//
uint8_t ui_ma[10] = {1+2+4+16+128+64, 4+16, 1+4+8+128+64, 1+4+8+16+64, 2+4+8+16, 1+2+8+16+64, 1+2+8+16+128+64, 1+4+16, 1+2+4+8+16+128+64, 1+2+4+8+16+64};
uint8_t geld_ma[8] = {200,100,50,20,10,5,2,1};

void init_ui(void){
	//PC0-7 and PD2 is uln2803 and transistor
	DDRC=255;
	PORTC=0;
	DDRD|=(1<<PD2);
	PORTD|=((1<<PD2)|(1<<PD6)|(1<<PD4)|(1<<PD3));
	// PD7 is 164 and-input
	// PD5 is 164 clk 
	DDRD|=((1<<PD7)|(1<<PD5));
	PORTD&=~((1<<PD7)|(1<<PD5));
	// led-array 6 5 4 3 2 1 none PD2/extraled 
}

void set_ui_darlington_resistor(uint8_t a, uint8_t extrab){
	PORTC=a;
	if (extrab & 0x01) PORTD&=~((1<<PD2)); else PORTD|=((1<<PD2));
}

void set_ui_74164_bit(uint8_t a){
	for(uint8_t i=0;i<8;i++){
		PORTD&=~((1<<PD7));
		if (a==i) PORTD|=((1<<PD7)); else PORTD&=~((1<<PD7));
		_NOP();
		PORTD&=~((1<<PD5));
		_NOP();
		PORTD|=((1<<PD5));
		_NOP();
	}
	
}

void set_ui(uint32_t aik){
		ui[1]=ui_ma[aik%10];
		aik/=10;
		ui[2]=ui_ma[aik%10];
		aik/=10;
		ui[3]=ui_ma[aik%10]|0x20;
		aik/=10;
		ui[4]=(aik==0)?0:ui_ma[aik%10];
		aik/=10;
		ui[5]=(aik==0)?0:ui_ma[aik%10];
		aik/=10;
		ui[6]=(aik==0)?0:ui_ma[aik%10];
}

#define MAX_GELD 32

#define MOTOR_STATE 0
#define DEBUG_STATE 1
#define AUTO_STOP 2

int main(void)
{


	uint8_t cur_164_bit=0;
	uint8_t old_ui_input=0;
	uint8_t ui_input=255;

	uint16_t count=0;

	uint8_t button_on_released=0;
	uint8_t button_on_pressed=0;
	uint8_t button_clear_released=0;
	uint8_t button_clear_pressed=0;
	uint8_t button_euro_released=0;
	uint8_t button_euro_pressed=0;
	uint8_t button_ec_released=0;
	uint8_t button_ec_pressed=0;

	init_sound();
	init_ui();
	DDRB|=(1<<PB2);
	PORTB|=(1<<PB2);
	//piep(snd_tetris);
	set_motor_speed(7);

	DDRA=0;
	PORTA=255;


	PCMSK0|=(1<<PCINT7);
	PCICR|=(1<<PCIE0);
	TCCR1B = (1<<CS10);
	sei();

	_delay_ms(500);
	//money auf null setzen
	geld[0]=0;
	geld[1]=0;
	geld[2]=0;
	geld[3]=0;
	geld[4]=0;
	geld[5]=0;
	geld[6]=0;
	geld[7]=0;
	old_stop_geld[0]=0;
	old_stop_geld[1]=0;
	old_stop_geld[2]=0;
	old_stop_geld[3]=0;
	old_stop_geld[4]=0;
	old_stop_geld[5]=0;
	old_stop_geld[6]=0;
	old_stop_geld[7]=0;


	while(1){

		set_ui_darlington_resistor(0,0);
		set_ui_74164_bit(cur_164_bit);
		set_ui_darlington_resistor(
			(((ui[0]>>cur_164_bit) & 0x01) << 0) | 
			(((ui[1]>>cur_164_bit) & 0x01) << 1) | 
			(((ui[2]>>cur_164_bit) & 0x01) << 2) | 
			(((ui[3]>>cur_164_bit) & 0x01) << 3) |
			(((ui[4]>>cur_164_bit) & 0x01) << 4) |
			(((ui[5]>>cur_164_bit) & 0x01) << 5) |
			(((ui[6]>>cur_164_bit) & 0x01) << 6) |
			(((ui[7]>>cur_164_bit) & 0x01) << 7),
			ui[8]>>cur_164_bit & 0x01
		);
		cur_164_bit=(cur_164_bit+1)%8;


		ui_input&=PIND;
		count++;
		if(count>500){
			ui_input=~ui_input;
			
			button_on_pressed= ((ui_input>>PD6) & 0x01) > ((old_ui_input>>PD6) & 0x01);
			button_on_released= ((ui_input>>PD6) & 0x01) < ((old_ui_input>>PD6) & 0x01);
			button_clear_pressed= ((ui_input>>PD4) & 0x01) > ((old_ui_input>>PD4) & 0x01);
			button_clear_released= ((ui_input>>PD4) & 0x01) < ((old_ui_input>>PD4) & 0x01);
			button_euro_pressed= ((ui_input>>PD3) & 0x01) > ((old_ui_input>>PD3) & 0x01);
			button_euro_released= ((ui_input>>PD3) & 0x01) < ((old_ui_input>>PD3) & 0x01);
			button_ec_pressed= button_clear_pressed && button_euro_pressed;
			button_ec_released= button_clear_released && button_euro_released;
			button_clear_pressed=button_clear_pressed & ~button_ec_pressed;
			button_euro_pressed=button_euro_pressed & ~button_ec_pressed;
			button_clear_released=button_clear_released & ~button_ec_released;
			button_euro_released=button_euro_released & ~button_ec_released;
			
			old_ui_input=ui_input;
			ui_input=255;
			count=0;
		}else{
			button_on_pressed=0;
			button_on_released=0;
			button_clear_pressed=0;
			button_clear_released=0;
			button_euro_pressed=0;
			button_euro_released=0;
			button_ec_pressed=0;
			button_ec_released=0;
		}

		
		if ((state>>AUTO_STOP) & 0x01){ 
			for(uint8_t tmp_i=0;tmp_i<8;tmp_i++){
				if((geld[tmp_i]-old_stop_geld[tmp_i])>MAX_GELD){
					state|=(1<<MOTOR_STATE);
					ui[8]&=~(1<<S);
					set_motor_speed(7);
					ui[8]|=1<<Q;
					old_stop_geld[tmp_i]=geld[tmp_i];
					zeig=tmp_i;
					
				}
			}
		}


		if (zeig==8){
			ui[0]=255;
			set_ui(
				(uint32_t)(geld[0])*(uint32_t)(200) + 
				(uint32_t)(geld[1])*(uint32_t)(100) + 
				(uint32_t)(geld[2])*(uint32_t)(50) + 
				(uint32_t)(geld[3])*(uint32_t)(20) + 
				(uint32_t)(geld[4])*(uint32_t)(10) + 
				(uint32_t)(geld[5])*(uint32_t)(5) + 
				(uint32_t)(geld[6])*(uint32_t)(2) + 
				(uint32_t)(geld[7])*(uint32_t)(1) 
				);
		}else{
			ui[0]=1<<2;
			switch(zeig){
				case 0 : ui[0]|=1<<7; break;
				case 1 : ui[0]|=1<<6; break;
				case 2 : ui[0]|=1<<5; break;
				case 3 : ui[0]|=1<<4; break;
				case 4 : ui[0]|=1<<3; break;
				case 5 : ui[0]|=1<<0; break;
				case 6 : ui[0]|=1<<1; break;
			}
			set_ui(geld[zeig]*geld_ma[zeig]);
			//set_ui(imw);
		}
		if (button_ec_pressed){
			if ((ui[8]>>Q)&0x01){
				ui[8]&=~(1<<Q);

				state&=~(1<<MOTOR_STATE);
				ui[8]|=1<<S;
				set_motor_speed(0);
			}else{
				if ((state>>AUTO_STOP) & 0x01){ 
					state&=~(1<<AUTO_STOP);
					ui[8]&=~(1<<BB);
				}else{
					state|=1<<AUTO_STOP;
					ui[8]|=1<<BB;
				}
			}
		}
		if (button_on_pressed && !((ui[8]>>Q)&0x01) ){
			if ((state>>MOTOR_STATE) & 0x01){
				state&=~(1<<MOTOR_STATE);
				ui[8]|=1<<S;
				set_motor_speed(0);
			}else{
				state|=(1<<MOTOR_STATE);
				ui[8]&=~(1<<S);
				set_motor_speed(7);
			}
		}
		if (button_euro_pressed){
			zeig=(zeig+1)%9;
		}
		if (button_clear_pressed){
			geld[0]=0;
			geld[1]=0;
			geld[2]=0;
			geld[3]=0;
			geld[4]=0;
			geld[5]=0;
			geld[6]=0;
			geld[7]=0;
			old_stop_geld[0]=0;
			old_stop_geld[1]=0;
			old_stop_geld[2]=0;
			old_stop_geld[3]=0;
			old_stop_geld[4]=0;
			old_stop_geld[5]=0;
			old_stop_geld[6]=0;
			old_stop_geld[7]=0;
		}

	}

}

// 4060-q7 mit PA7 verbinden!!

ISR(PCINT0_vect){
		imw=TCNT1;
		TCNT1=0;
		
		if ((imw<2101) && (imw>2040)){
			ir_max|=(PINA&0x7f);
			imw_can_set=1;
		}
		if((imw>2112) && (imw_can_set)){
			switch (ir_max){
				case 0x00: geld[7]++; break;
				case 0x01: geld[6]++; break;
				case 0x03: geld[4]++; break;
				case 0x07: geld[5]++; break;
				case 0x0f: geld[3]++; break;
				case 0x1f: geld[1]++; break;
				case 0x3f: geld[2]++; break;
				case 0x7f: geld[0]++; break;
			}
			ir_max=0;
			imw_can_set=0;
		}


}
