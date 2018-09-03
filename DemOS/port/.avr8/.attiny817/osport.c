/******************************************************************************

    @file    DemOS: osport.c
    @author  Rajmund Szymanski
    @date    03.09.2018
    @brief   DemOS port file for ATtiny817 uC.

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

ISR( TCA0_OVF_vect )
{
	TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;
	sys_counter++;
}

/* --------------------------------------------------------------------------------------------- */

void sys_init( void )
{
	TCA0.SINGLE.PER     = F_CPU / 1000 / 16 - 1;
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;
	TCA0.SINGLE.CTRLB   = TCA_SINGLE_WGMODE_NORMAL_gc;
	TCA0.SINGLE.CTRLA   = TCA_SINGLE_CLKSEL_DIV16_gc
	                    | TCA_SINGLE_ENABLE_bm;
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
