/* 
 * 2018, Laurent Ghigonis <laurent@gouloum.fr>
 */

#include <avr/io.h>
#include <util/atomic.h>
#include "ps2.h"
#include "conf.h"

#define PS2_BUF_LEN 128

static void _decode(unsigned char);

unsigned char _buf[PS2_BUF_LEN]; /* only used within ATOMIC_BLOCK */
uint8_t _bufbeg = 0; /* only used within ATOMIC_BLOCK */
uint8_t _bufend = 0; /* only used within ATOMIC_BLOCK */

extern unsigned char _unshifted[67][2];
extern unsigned char _shifted[67][2];

void
ps2_init(void)
{
	PS2_DDR &= ~(1 << PS2_PIN_CLOCK); /* set pin CLOCK as input */
	PS2_DDR &= ~(1 << PS2_PIN_DATA);  /* set pin DATA as input */
	PS2_PORT |= (1 << PS2_PIN_CLOCK); /* enable pin CLOCK pull-up */
	PS2_PORT |= (1 << PS2_PIN_DATA);  /* enable pin DATA pull-up */
	PCICR |= (1 << PS2_PCIE);	  /* set PCIE1 to enable PCMSK1 scan */
	PCMSK1 |= (1 << PS2_PCINT);	  /* set PCINT8 to trigger interrupts */
}

/* ps2_read - called from interrupt vector */
void
ps2_read(void)
{
	static unsigned char data;
	static unsigned char bitcount = 11;

	/* bit 3 to 10 is data, start, stop & parity bis are ignored */
	if((bitcount < 11) && (bitcount > 2)) {
	        /* data bit */
		data = (data >> 1);      
		if ((PINC & (1 << PS2_PIN_DATA))) /* read DATA pin */
			data = data | 0x80; /* store '1' */
		else
			data = data & 0x7f; /* store '0' */
	}      
	if (--bitcount == 0) {
		/* all bits received */
		_decode(data);
		bitcount = 11;
	} 
}

/* ps2_buf_push - append a char to the pressed keys buffer
 * typically called in PCINT interrupt */
void
ps2_buf_push(unsigned char c)
{
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		_buf[_bufend] = c;
		if (_bufend+1 == PS2_BUF_LEN)
			_bufend = 0;
		else
			_bufend++;
		if (_bufend == _bufbeg) { /* buffer full, remove a char */
			if (_bufbeg+1 == PS2_BUF_LEN)
				_bufbeg = 0;
			else
				_bufbeg++;
		}
	}
}

/* ps2_buf_pull - fetch a char from the pressed keys buffer
 * typically called outside of interrupts */
unsigned char
ps2_buf_pull(void)
{
	unsigned char c;

	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		if (_bufbeg == _bufend)
			return 0;
		c = _buf[_bufbeg];
		if (_bufbeg+1 == PS2_BUF_LEN)
			_bufbeg = 0;
		else
			_bufbeg++;
	}
	return c;
}

/* ps2_buf_empty - empty pressed keys buffer */
void
ps2_buf_empty(void)
{
	_bufbeg = 0;
	_bufend = 0;
}

/* decode PS2 scan code */
static void
_decode(unsigned char sc)
{
	static unsigned char is_up = 0;
	static unsigned char shift = PS2_SHIFT_DEFAULT;
	unsigned char i;
 
	if (!is_up) {
		switch (sc) {
		case 0xF0 : /* up-key */
			is_up = 1;
			break;
		case 0x12 : /* left shift */
			shift = 1;
			break;
		case 0x59 : /* right shift */
			shift = 1;
			break;
		default:
#if PS2_DEBUG
			ps2_buf_push(sc);
#else
			if(!shift) {
				for(i = 0; _unshifted[i][0]!=sc && _unshifted[i][0]; i++);
				if (_unshifted[i][0] == sc) {
					ps2_buf_push(_unshifted[i][1]);
				}                                       
			} else {
				for(i = 0; _shifted[i][0]!=sc && _shifted[i][0]; i++);
				if (_shifted[i][0] == sc) {
					ps2_buf_push(_shifted[i][1]);
				}
			}
#endif /* PS2_DEBUG */
			break;
		}
	} else {
		is_up = 0; /* two 0xF0 in a row not allowed */
		switch (sc) {
		case 0x12 : /* left shift */
			shift = 0;
			break;
		case 0x59 : /* right shift */
			shift = 0;
			break;
		}
	}
}                                               

