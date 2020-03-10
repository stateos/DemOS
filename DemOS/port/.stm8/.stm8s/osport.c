/******************************************************************************

    @file    DemOS: osport.c
    @author  Rajmund Szymanski
    @date    10.03.2020
    @brief   DemOS port file for STM8S uC.

 ******************************************************************************

   Copyright (c) 2018-2020 Rajmund Szymanski. All rights reserved.

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

INTERRUPT_HANDLER(TIM4_UPD_OVF_IRQHandler, 23)
{
	TIM4->SR1 = (uint8_t) ~TIM4_SR1_UIF; // clear timer's status register
	sys_counter++;
}

/* --------------------------------------------------------------------------------------------- */

void sys_init( void )
{
	CLK->CKDIVR = 0;
	CLK->ECKR  |= CLK_ECKR_HSEEN; while ((CLK->ECKR & CLK_ECKR_HSERDY) == 0);
	CLK->SWCR  |= CLK_SWCR_SWEN;
	CLK->SWR    = 0xB4; /* HSE */ while ((CLK->SWCR & CLK_SWCR_SWBSY)  == 1);

	TIM4->PSCR  = 6;             // Prescaler:      16.000.000 /  64 = 250.000
	TIM4->ARR   = 249;           // Auto-reload value: 250.000 / 250 =   1.000
	TIM4->IER  |= TIM4_IER_UIE;  // Enable interrupt
	TIM4->CR1  |= TIM4_CR1_CEN;  // Enable timer

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
