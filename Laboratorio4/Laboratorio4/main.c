/*
* Laboratorio4.c
* Created: 3/04/2025 23:01:04
* Author: Alma Mata
* Description: 
*/
/****************************************/
// Encabezado (Libraries)
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

/****************************************/
// Variables globales
volatile uint8_t contador = 0;
uint8_t Tabla7seg[] = {0x40, 0x79, 0x24, 0x30, 0x19, 0x12, 0x02, 0x78, 0x00, 0x10, 0x08, 0x03, 0x46, 0x21, 0x06, 0x0E};

#define multiplexeo ((1 << PORTB0) | (1 << PORTB1))
#define MASK_OUTB ((1 << PORTB2) | (1 << PORTB3) | (1 << PORTB4) | (1 << PORTB5))
#define MASK_OUTC ((1 << PORTC0) | (1 << PORTC1) | (1 << PORTC2) | (1 << PORTC3))

// Function prototype
void setup();
void update_OUT();

/****************************************/
// Main Function
int main(void)
{
    setup();
    while (1) 
    {
		update_OUT();
		
    }
	return 0;
}

/****************************************/
// NON-Interrupt subroutines
void setup()
{
	cli();
	// Configuración de puertos
	DDRB |= MASK_OUTB;
	DDRC |= MASK_OUTC;
	DDRC &= ~((1 << PORTC4) | (1 << PORTC5));
	PORTC |= (1 << PORTC4) | (1 << PORTC5);
	
	// Habilitar interrupciones en el puerto C
	PCMSK1 |= (1 << PCINT12) | (1 << PCINT13);
	PCICR |= (1 << PCIE1);
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
	if(!(PINC & 0b00010000)) //Si es PB1 se resta al contador
	{
		contador++;
	}//Fin Resta
	if(!(PINC & 0b00100000)){ // Si es PB0 se añade al contador
		contador--;
	}
}