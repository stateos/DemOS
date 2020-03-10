/******************************************************************************

    @file    DemOS: osport.c
    @author  Rajmund Szymanski
    @date    10.03.2020
    @brief   DemOS port file for STM32F4 uC.

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

#ifndef USE_HAL_DRIVER

volatile
cnt_t sys_counter = 0;

#endif

/* --------------------------------------------------------------------------------------------- */

#ifndef USE_HAL_DRIVER

void SysTick_Handler( void )
{
	SysTick->CTRL;
	sys_counter++;
}

#endif

/* --------------------------------------------------------------------------------------------- */

void sys_init( void )
{
#ifndef USE_HAL_DRIVER
	SysTick_Config(SystemCoreClock/1000);
#endif
}

/* --------------------------------------------------------------------------------------------- */

cnt_t sys_time( void )
{
	cnt_t cnt;
#ifndef USE_HAL_DRIVER
	cnt = sys_counter;
#else
	cnt = HAL_GetTick();
#endif
	return cnt;
}

/* --------------------------------------------------------------------------------------------- */
