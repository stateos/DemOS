/******************************************************************************

    @file    DemOS: osport.c
    @author  Rajmund Szymanski
    @date    03.09.2018
    @brief   DemOS port file for ST7 uC.

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

/* --------------------------------------------------------------------------------------------- */

volatile
cnt_t sys_counter = 0;

/* --------------------------------------------------------------------------------------------- */

@interrupt
void TIMERA_IRQHandler(void)
{
	TASR;     // clear OCF1, step 1
	TAOC1R;   // clear OCF1, step 2
	TACR = 0; // reset free running counter
	sys_counter++;
}

/* --------------------------------------------------------------------------------------------- */

void sys_init( void )
{
	TAOC1R = 2000; // 8000000 / 4 / 2000 = 1000 Hz
	TAOC2R =   -6; // freeze channel 2 of timer A
	TACR2  =    0; // fCPU / 4
	TACR1 |= TACR1_OCIE_OR;

	enableInterrupts();
}

/* --------------------------------------------------------------------------------------------- */

cnt_t sys_time( void )
{
	cnt_t cnt;
	disableInterrupts();
	cnt = sys_counter;
	enableInterrupts();
	return cnt;
}

/* --------------------------------------------------------------------------------------------- */
