/*
* Laboratorio4.c
* Created: 3/04/2025 23:01:04
* Author: Alma Mata
* Description: El prelaboratorio es un contador de 8 bits, el laboratorio es un contador que usa un ADC.
*		El post laboratorio es una alarma que se activa cuando el ADC sobre pasa al contador de 8 bits.
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
uint8_t display_actual = 0b0001;
uint8_t out_PORTD = 0x40;
uint8_t valor_ADC = 0;
uint16_t valor_ADC_filtrado = 0;
uint8_t Tabla7seg[] = {0x40, 0x79, 0x24, 0x30, 0x19, 0x12, 0x02, 0x78, 0x00, 0x10, 0x08, 0x03, 0x46, 0x21, 0x06, 0x0E};

#define DISPLAY1 (1 << PORTB0)
#define DISPLAY2 (1 << PORTB1)
#define DISPLAY3 (1 << PORTB2)
#define pin_ALARMA (1 << PORTB3)

// Function prototype
void setup();
void comparar_valor();

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
		// Procesar ADC
		valor_ADC = leer_ADC();
		
		// Aplicar filtro de media móvil exponencial
		valor_ADC_filtrado = (valor_ADC_filtrado * 7 + valor_ADC) / 8;

		PORTB = 0x00;
		if (display_actual == 0b0001)
		{
			PORTD = Tabla7seg[valor_ADC_filtrado & 0x0F];
			PORTB |= DISPLAY1;
		}
		if (display_actual == 0b0010)
		{
			PORTD = Tabla7seg[(valor_ADC_filtrado & 0xF0)>>4];
			PORTB |= DISPLAY2;
		}
		if (display_actual == 0b0100)
		{
			PORTD = contador;
			PORTB |= DISPLAY3;
		}
		comparar_valor();
	}
	return 0;
}

/****************************************/
// NON-Interrupt subroutines
void setup()
{
	cli();
	//SPCR = 0x00; // Asegura que el SPI esté desactivado
	// Configuración de puertos
	DDRB = 0xFF;
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
	ADMUX = (1 << REFS0) | (1 << ADLAR) | 7; // ADC7 (A7)
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
	sei();
}

void comparar_valor()
{
	// Solo encender la alarma si el valor filtrado supera el contador con margen
	if (contador < (valor_ADC_filtrado & 0xFF)) // cast a 8 bits por seguridad
	{
		PORTB |= pin_ALARMA;
	}
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
	if (display_actual == 0b0001)
	{
		display_actual = 0b0010;
	}
	else{ if (display_actual == 0b0010)
		{
			display_actual = 0b0100;
		}
		else {
			display_actual = 0b0001;
		}
	}
}