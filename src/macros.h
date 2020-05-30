#pragma once

#define OUTPUT 1
#define INPUT 0
#define HIGH 1
#define LOW 0

#define PORTB_MODE(pin, mode) mode ? DDRB |= (1<<pin) : DDRB&=~(1<<pin)
#define PORTC_MODE(pin, mode) mode ? DDRC |= (1<<pin) : DDRC&=~(1<<pin)
#define PORTD_MODE(pin, mode) mode ? DDRD |= (1<<pin) : DDRD&=~(1<<pin)

#define PORTB_READ(pin) (PINB & B00000001 << pin) > 0       //Digital read analog
#define PORTC_READ(pin) (PINC & B00000001 << pin) > 0       //Digital read analog
#define PORTD_READ(pin) (PIND & B00000001 << pin) > 0       //Digital read analog

#define PORTB_WRITE(pin, val) val ? PORTB |= (B00000001 << pin) : PORTB &=~(B00000001 << pin)
#define PORTC_WRITE(pin, val) val ? PORTC |= (B00000001 << pin) : PORTC &=~(B00000001 << pin)
#define PORTD_WRITE(pin, val) val ? PORTD |= (B00000001 << pin) : PORTD &=~(B00000001 << pin)