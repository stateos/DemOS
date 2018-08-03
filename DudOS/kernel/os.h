/******************************************************************************

    @file    DudOS: os.h
    @author  Rajmund Szymanski
    @date    03.08.2018
    @brief   This file provides set of functions for DudOS.

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

#ifndef __DUDOS

#define __DUDOS_MAJOR        0
#define __DUDOS_MINOR        1
#define __DUDOS_BUILD        0

#define __DUDOS        ((((__DUDOS_MAJOR)&0xFFUL)<<24)|(((__DUDOS_MINOR)&0xFFUL)<<16)|((__DUDOS_BUILD)&0xFFFFUL))

#define __DUDOS__           "DudOS v" STRINGIZE(__DUDOS_MAJOR) "." STRINGIZE(__DUDOS_MINOR) "." STRINGIZE(__DUDOS_BUILD)

#define STRINGIZE(n) STRINGIZE_HELPER(n)
#define STRINGIZE_HELPER(n) #n

/* -------------------------------------------------------------------------- */

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "osport.h"

#ifdef __cplusplus
extern "C" {
#endif

/* System =================================================================== */

#define MSEC                  1      // time multiplier given in milliseconds
#define SEC                1000      // time multiplier given in seconds
#define MIN               60000      // time multiplier given in minutes
#ifndef INFINITE
#define INFINITE           ( ~0UL )  // time value for infinite wait
#endif
/* -------------------------------------------------------------------------- */
extern volatile
cnt_t   sys_counter;                 // port variable - system time counter
void    sys_init ( void );           // port function - initialize the system timer
cnt_t   sys_time ( void );           // port function - get current value of the system timer (in milliseconds)

#ifdef  USE_GOTO

/* Task ( goto / label ) ==================================================== */

typedef void * tag_t;

#define TSK_CONCATENATE(tag)            Line ## tag
#define TSK_LABEL(tag)                  TSK_CONCATENATE(tag)
#define TSK_STATE(tag)               && TSK_LABEL(tag)
#define TSK_BEGIN()                     if (sys_task->state) goto *sys_task->state
#define TSK_END()

#else//!USE_GOTO

/* Task ( switch / case ) =================================================== */

typedef intptr_t tag_t;

#define TSK_LABEL(tag)                  /* falls through */ case tag
#define TSK_STATE(tag)                  tag
#define TSK_BEGIN()                     switch (sys_task->state) { case 0:
#define TSK_END()                       }

#endif//USE_GOTO

/* Task ===================================================================== */
// definition of task function
typedef void fun_t( void );
// definition of task state
typedef enum   __tid { ID_RIP = 0, ID_RDY, ID_DLY } tid_t;
// definition of task structure
typedef struct __tsk { cnt_t tmr; tid_t id; tag_t state; fun_t *function; struct __tsk *next; } tsk_t;

extern
tsk_t * sys_task;                    // system variable - current task
void    tsk_start( tsk_t *tsk );     // system function - make task ready to execute

/* -------------------------------------------------------------------------- */
// for internal use; start the system scheduler
#define SYS_START()                 do { if ((sys_task = sys_task->next)->id == ID_RDY) sys_task->function(); } while(1)

/* -------------------------------------------------------------------------- */
// initialize the system timer and start the system scheduler
#define sys_start()                 do { sys_init(); SYS_START();                                             } while(0)

/* -------------------------------------------------------------------------- */
// initialize a task structure with function (fun)
#define TSK_INIT(fun)                   { 0, ID_RIP, 0, fun, 0 }
// define and initialize the task (tsk) with function (fun)
#define OS_TSK(tsk, fun)                tsk_t tsk[] = { TSK_INIT(fun) }
// define and initialize the task (tsk); function body must be defined immediately below
#define OS_TSK_DEF(tsk)                 void tsk ## __fun( void ); \
                                        OS_TSK(tsk, tsk ## __fun); \
                                        void tsk ## __fun( void )
// define, initialize and start the task (tsk); function body must be defined immediately below
#define OS_TSK_START(tsk)               void tsk ## __fun( void ); \
                                        OS_TSK(tsk, tsk ## __fun); \
           __attribute__((constructor)) void tsk ## __run( void ) { tsk_start(tsk); } \
                                        void tsk ## __fun( void )
/* -------------------------------------------------------------------------- */
// for internal use; wait while the condition (cnd) is true
#define TSK_WHILE(cnd)                  sys_task->state = TSK_STATE(__LINE__); TSK_LABEL(__LINE__): if (cnd) return; (void)0
// for internal use; pass control to the next ready task if the condition (cnd) is true
#define TSK_YIELD(cnd)                  sys_task->state = TSK_STATE(__LINE__); if (cnd) return; TSK_LABEL(__LINE__): (void)0
/* -------------------------------------------------------------------------- */
// check whether the task (tsk) is the current task
#define tsk_self(tsk)                 ( (tsk) == sys_task )
// necessary prologue of the task
#define tsk_begin()                     TSK_BEGIN(); do {                                                          } while(0)
// necessary epilogue of the task
#define tsk_end()                       TSK_END();   do { tsk_again();                                             } while(0)
// wait while the condition (cnd) is true
#define tsk_waitWhile(cnd)         do { TSK_WHILE(cnd);                                                            } while(0)
// wait while the condition (cnd) is false
#define tsk_waitUntil(cnd)         do { tsk_waitWhile(!(cnd));                                                     } while(0)
// start new or restart previously stopped task (tsk) with function (fun)
#define tsk_startFrom(tsk, fun)    do { if ((tsk)->id == ID_RIP) { (tsk)->function = (fun); tsk_start(tsk); }      } while(0)
// wait while the task (tsk) is working
#define tsk_join(tsk)              do { tsk_waitUntil((tsk)->id == ID_RIP);                                        } while(0)
// start task (tsk) and wait for the end of execution of (tsk)
#define tsk_call(tsk)              do { tsk_start(tsk); tsk_join(tsk);                                             } while(0)
// start new or restart previously stopped task (tsk) with function (fun) and wait for the end of execution of (tsk)
#define tsk_callFrom(tsk, fun)     do { tsk_startFrom(tsk, fun); tsk_join(tsk);                                    } while(0)
// restart the current task from the initial state
#define tsk_again()                do { sys_task->state = 0; return;                                               } while(0)
// stop the current task; it will no longer be executed
#define tsk_stop()                 do { sys_task->id = ID_RIP; return;                                             } while(0)
// stop the task (tsk); it will no longer be executed
#define tsk_kill(tsk)              do { (tsk)->id = ID_RIP; if (tsk_self(tsk)) return;                             } while(0)
// pass control to the next ready task
#define tsk_yield()                do { TSK_YIELD(true);                                                           } while(0)
// restart the current task with function (fun)
#define tsk_flip(fun)              do { sys_task->function = (fun); tsk_again();                                   } while(0)
// delay execution of current task for given duration of time (dly)
#define tsk_sleepFor(dly)          do { tmr_waitFor(&sys_task->tmr, dly);                                          } while(0)
// delay execution of current task for given duration of time (dly) from the end of the previous countdown
#define tsk_sleepNext(dly)         do { tmr_waitNext(&sys_task->tmr, dly);                                         } while(0)
// delay execution of current task until given timepoint (tim)
#define tsk_sleepUntil(tim)        do { tmr_waitUntil(&sys_task->tmr, tim);                                        } while(0)
// delay indefinitely execution of current task
#define tsk_sleep()                do { tsk_sleepFor(INFINITE);                                                    } while(0)
// delay execution of current task for given duration of time (dly)
#define tsk_delay(dly)             do { tsk_sleepFor(dly);                                                         } while(0)
// suspend execution of the ready task (tsk)
#define tsk_suspend(tsk)           do { if ((tsk)->id == ID_RDY) { (tsk)->id = ID_DLY; TSK_YIELD(tsk_self(tsk)); } } while(0)
// resume execution of the suspended task (tsk)
#define tsk_resume(tsk)            do { if ((tsk)->id == ID_DLY) { (tsk)->id = ID_RDY; }                           } while(0)

/* Timer ==================================================================== */
// definition of timer
typedef cnt_t tmr_t;

/* -------------------------------------------------------------------------- */
// define and initialize the timer (tmr)
#define OS_TMR(tmr)                     tmr_t tmr[] = { 0 }
/* -------------------------------------------------------------------------- */
// check if the timer (tmr) finishes countdown for given duration of time (dly)
#define tmr_expiredFor(tmr, dly)      ( sys_time() - *(tmr) + 1 > (dly) )
// check if the timer (tmr) finishes countdown until given timepoint (tim)
#define tmr_expiredUntil(tmr, tim)    ( tmr_expiredFor(tmr, (tim) - *(tmr)) )
// start the timer (tmr)
#define tmr_start(tmr)             do { *(tmr) = sys_time();                                                       } while(0)
// wait until the timer (tmr) finishes countdown for given duration of time (dly)
#define tmr_waitFor(tmr, dly)      do { tmr_start(tmr); tsk_waitUntil(tmr_expiredFor(tmr, dly));  *(tmr) += (dly); } while(0)
// wait until the timer (tmr) finishes countdown for given duration of time (dly) from the end of the previous countdown
#define tmr_waitNext(tmr, dly)     do {                 tsk_waitUntil(tmr_expiredFor(tmr, dly));  *(tmr) += (dly); } while(0)
// wait until the timer (tmr) finishes countdown until given timepoint (tim)
#define tmr_waitUntil(tmr, tim)    do { tmr_start(tmr); tsk_waitUntil(tmr_expiredUntil(tmr, tim)); *(tmr) = (tim); } while(0)

/* Binary semaphore ========================================================= */
// definition of binary semaphore
typedef uint_fast8_t sem_t;

/* -------------------------------------------------------------------------- */
// define and initialize the binary semaphore (sem) with initial value (ini)
#define OS_SEM(sem, ini)                sem_t sem[] = { ini }
/* -------------------------------------------------------------------------- */
// wait for the semaphore (sem)
#define sem_wait(sem)              do { tsk_waitUntil(*(sem)); *(sem) = 0; } while(0)
// release the semaphore (sem)
#define sem_give(sem)              do { *(sem) = 1;                        } while(0)

/* Protected signal ========================================================= */
// definition of protected signal
typedef uint_fast8_t sig_t;

/* -------------------------------------------------------------------------- */
// define and initialize the protected signal (sig)
#define OS_SIG(sig)                     sig_t sig[] = { 0 }
/* -------------------------------------------------------------------------- */
// wait for the signal (sig)
#define sig_wait(sig)              do { tsk_waitUntil(*(sig)); } while(0)
// release the signal (sig)
#define sig_give(sig)              do { *(sig) = 1;            } while(0)
// reset the signal (sig)
#define sig_clear(sig)             do { *(sig) = 0;            } while(0)

/* Event ==================================================================== */
// definition of event
typedef uintptr_t evt_t;

/* -------------------------------------------------------------------------- */
// define and initialize the event (evt)
#define OS_EVT(evt)                     evt_t evt[] = { 0 }
/* -------------------------------------------------------------------------- */
// get a value of the event (evt)
#define evt_take(evt)                 ( *(evt) )
// wait for a the new value of the event (evt)
#define evt_wait(evt)              do { *(evt) = 0; tsk_waitUntil(*(evt)); } while(0)
// set a new value (val) of the event (evt)
#define evt_give(evt, val)         do { *(evt) = (val);                    } while(0)

/* Mutex ==================================================================== */
// definition of mutex
typedef tsk_t *mtx_t;

/* -------------------------------------------------------------------------- */
// define and initialize the mutex (mtx)
#define OS_MTX(mtx)                     mtx_t mtx[] = { 0 }
/* -------------------------------------------------------------------------- */
// wait for the mutex (mtx)
#define mtx_wait(mtx)              do { tsk_waitWhile(*(mtx)); *(mtx) = sys_task; } while(0)
// release previously owned mutex (mtx)
#define mtx_give(mtx)              do { if (*(mtx) == sys_task) *(mtx) = 0;       } while(0)

/* -------------------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

#endif//__DUDOS
