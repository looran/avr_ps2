/* avr_ps2 example - turn on led on PC2 with <enter>, off with <space>
 * see configuration in conf.h */

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "ps2.h"
#include "ps2_keymap_en.h"
#include "conf.h"

int
main(void)
{
	unsigned char c;

	ps2_init();
	DDRC |= (1 << PC2); /* set PC2 as output, for LED */
	sei();

	while (1) {
		c = ps2_buf_pull();
		if (c != 0) {
			if (c == '\r')
				PORTC |= (1 << PC2); /* LED on */
			else if (c == ' ')
				PORTC &= ~(1 << PC2); /* LED off */
		}
		_delay_ms(100);
	}
}

ISR (PS2_PCINT_vect) { /* PS2 interrupt */
	static volatile uint8_t last = 0xFF; /* default high because pull-up */
	uint8_t changed;

	changed = PS2_PIN ^ last;
	last = PS2_PIN;

	if (changed & (1 << PS2_PIN_CLOCK)) { /* CLOCK changed */
		if ((PS2_PIN & (1 << PS2_PIN_CLOCK)) == 0) /* CLOCK LOW */
			ps2_read();
	}
}

