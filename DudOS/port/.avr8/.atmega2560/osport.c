/******************************************************************************

    @file    DudOS: osport.c
    @author  Rajmund Szymanski
    @date    28.07.2018
    @brief   DudOS port file for ATtiny817 uC.

 ******************************************************************************

   Copyright (c) 2018 Rajmund Szymanski. All rights reserved.

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to
   deal in the Software without restriction, including without limitation the
   rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
   sell copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
   IN THE SOFTWARE.

 ******************************************************************************/

#include "os.h"
#include <avr/interrupt.h>

/* --------------------------------------------------------------------------------------------- */

volatile
cnt_t sys_counter = 0;

/* --------------------------------------------------------------------------------------------- */

ISR( TIMER1_OVF_vect )
{
	sys_counter++;
}

/* --------------------------------------------------------------------------------------------- */

void sys_init( void )
{
	OCR1AH = (F_CPU / 1000 / 64 - 1) >> 8;
	OCR1AL = (F_CPU / 1000 / 64 - 1);

	TCCR1B = (1 << CS11) | (1 << CS10);
	TCCR1A = (1 << COM1A1) | (1 << COM1A0);
	TIMSK1 = (1 << OCIE1A);

	sei();
}

/* --------------------------------------------------------------------------------------------- */

cnt_t sys_time( void )
{
	cnt_t cnt;
	cli();
	cnt = sys_counter;
	sei();
	return cnt;
}

/* --------------------------------------------------------------------------------------------- */
