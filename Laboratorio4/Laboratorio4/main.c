/*
* Laboratorio4.c
* Created: 3/04/2025 23:01:04
* Author: Alma Mata
* Description: 
*/
/****************************************/
// Encabezado (Libraries)
#define F_CPU 16000000	//Frecuencia es 16Mhz
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

/****************************************/
// Variables globales
volatile uint8_t contador = 0;
uint8_t display_actual = 0;
uint8_t out_PORTD = 0x40;
uint8_t valor_ADC = 0;
uint8_t Tabla7seg[] = {0x40, 0x79, 0x24, 0x30, 0x19, 0x12, 0x02, 0x78, 0x00, 0x10, 0x08, 0x03, 0x46, 0x21, 0x06, 0x0E};

#define DISPLAY1 (1 << PORTB0)	
#define DISPLAY2 (1 << PORTB1)
#define MASK_OUTB ((1 << PORTB2) | (1 << PORTB3) | (1 << PORTB4) | (1 << PORTB5))
#define MASK_OUTC ((1 << PORTC0) | (1 << PORTC1) | (1 << PORTC2) | (1 << PORTC3))

// Function prototype
void setup();
void update_OUT();

uint8_t leer_ADC() {
	ADCSRA |= (1 << ADSC);
	while (ADCSRA & (1 << ADSC));
	return ADCH;
}

/****************************************/
// Main Function
int main(void)
{
    setup();
    while (1) 
    {
		update_OUT();
		
		// Procesar ADC
		valor_ADC = leer_ADC();
		if (display_actual == 0)
		{
			PORTB = 0x00;
			PORTD = Tabla7seg[valor_ADC & 0x0F];
			PORTB |= DISPLAY1;
		}
		else{
			PORTB = 0x00;
			PORTD = Tabla7seg[(valor_ADC & 0xF0)>>4];
			PORTB |= DISPLAY2;
		}
    }
	return 0;
}

/****************************************/
// NON-Interrupt subroutines
void setup()
{
	cli();
	// Configuración de puertos
	DDRB = 0xFF;
	DDRC |= MASK_OUTC;
	DDRC &= ~((1 << PORTC4) | (1 << PORTC5));
	PORTC |= (1 << PORTC4) | (1 << PORTC5);
	DDRD = 0xFF;
	
	// Habilitar interrupciones en el puerto C
	PCMSK1 |= (1 << PCINT12) | (1 << PCINT13);
	PCICR |= (1 << PCIE1);
	
	// Habilitar interrupciones del TIMER0
	TCCR0A = 0x00;  // Modo Normal
	TCCR0B = (1 << CS01) | (1 << CS00);   //Prescaler 64
	TCNT0 = 131;  
	TIMSK0 = (1 << TOIE0);
	
	// Configuracion ADC
	ADMUX = 0;
	ADMUX |= (1<<REFS0)|(1<<ADLAR)|(1<<MUX2)|(1<<MUX1)|(1<<MUX0);
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
	sei();
}

void update_OUT()
{
	// Salida del contador en PORTB y PORTC
	PORTB = (PORTB & ~MASK_OUTB) | ((contador & 0x0F) << 2);
	PORTC = (PORTC & ~MASK_OUTC) | ((contador >> 4) & 0x0F);
}

/****************************************/
// Interrupt subroutines
ISR(PCINT1_vect)
{
	if(!(PINC & 0b00010000)) //Si es PC4 aumenta contador
	{
		contador++;
	}
	if(!(PINC & 0b00100000)) // Si es PC5 disminuye contador
	{ 
		contador--;
	}
}
ISR(TIMER0_OVF_vect)
{	
	display_actual = !display_actual;
}