/******************************************************************************

    @file    DudOS: os.h
    @author  Rajmund Szymanski
    @date    01.08.2018
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
cnt_t   sys_counter;                 // system variable - system time counter
void    sys_init ( void );           // port function - initialize the system timer
cnt_t   sys_time ( void );           // port function - get current value of the system timer (in milliseconds)
void    sys_start( void );           // system function - start the system scheduler

#ifdef  USE_GOTO

/* Task ( goto / label ) ==================================================== */

typedef void * tag_t;

#define TSK_CONCATENATE(tag)            Line ## tag
#define TSK_LABEL(tag)                  TSK_CONCATENATE(tag)
#define TSK_STATE(tag)               && TSK_LABEL(tag)
#define TSK_BEGIN()                     if (tsk_this->state) goto *tsk_this->state
#define TSK_END()

#else//!USE_GOTO

/* Task ( switch / case ) =================================================== */

typedef intptr_t tag_t;

#define TSK_LABEL(tag)                  /* falls through */ case tag
#define TSK_STATE(tag)                  tag
#define TSK_BEGIN()                     switch (tsk_this->state) { case 0:
#define TSK_END()                       }

#endif//USE_GOTO

/* Task ===================================================================== */

typedef void fun_t( void );

typedef enum   __tid { ID_RIP = 0, ID_RDY, ID_DLY } tid_t;

typedef struct __tsk { cnt_t tmr; tid_t id; tag_t state; fun_t *function; struct __tsk *next; } tsk_t;

extern
tsk_t * tsk_this;                    // system variable - current task

void    tsk_start( tsk_t *tsk );     // system function - make task ready to execute

/* -------------------------------------------------------------------------- */
#define TSK_INIT(fun)                   { 0, ID_RIP, 0, fun, 0 }

#define OS_TSK(tsk, fun)                tsk_t tsk[] = { TSK_INIT(fun) }

#define OS_TSK_DEF(tsk)                 void tsk ## __fun( void ); \
                                        OS_TSK(tsk, tsk ## __fun); \
                                        void tsk ## __fun( void )

#define OS_TSK_START(tsk)               void tsk ## __fun( void ); \
                                        OS_TSK(tsk, tsk ## __fun); \
           __attribute__((constructor)) void tsk ## __run( void ) { tsk_start(tsk); } \
                                        void tsk ## __fun( void )
/* -------------------------------------------------------------------------- */
//                                      for internal use
#define TSK_WHILE(cnd)                  tsk_this->state = TSK_STATE(__LINE__); TSK_LABEL(__LINE__): if (cnd) return; (void)0
//                                      for internal use
#define TSK_YIELD(cnd)                  tsk_this->state = TSK_STATE(__LINE__); if (cnd) return; TSK_LABEL(__LINE__): (void)0
/* -------------------------------------------------------------------------- */
//      tsk_begin                       necessary prologue of the task
#define tsk_begin()                     TSK_BEGIN(); do {                                                          } while(0)
//      tsk_end                         necessary epilogue of the task
#define tsk_end()                       TSK_END();   do { tsk_again();                                             } while(0)
//      tsk_self                        check whether the task (tsk) is the current task
#define tsk_self(tsk)                 ( (tsk) == tsk_this )
//      tsk_waitWhile                   wait while the condition (cnd) is true
#define tsk_waitWhile(cnd)         do { TSK_WHILE(cnd);                                                            } while(0)
//      tsk_waitUntil                   wait while the condition (cnd) is false
#define tsk_waitUntil(cnd)         do { tsk_waitWhile(!(cnd));                                                     } while(0)
//      tsk_startFrom                   start new or restart previously stopped task (tsk) with function (fun)
#define tsk_startFrom(tsk, fun)    do { if ((tsk)->id == ID_RIP) { (tsk)->function = (fun); tsk_start(tsk); }      } while(0)
//      tsk_join                        wait while the task (tsk) is working
#define tsk_join(tsk)              do { tsk_waitUntil((tsk)->id == ID_RIP);                                        } while(0)
//      tsk_call                        start task (tsk) and wait for the end of execution of (tsk)
#define tsk_call(tsk)              do { tsk_start(tsk); tsk_join(tsk);                                             } while(0)
//      tsk_callFrom                    start new or restart previously stopped task (tsk) with function (fun) and wait for the end of execution of (tsk)
#define tsk_callFrom(tsk, fun)     do { tsk_startFrom(tsk, fun); tsk_join(tsk);                                    } while(0)
//      tsk_again                       restart the current task from the initial state
#define tsk_again()                do { tsk_this->state = 0; return;                                               } while(0)
//      tsk_stop                        stop the current task; it will no longer be executed
#define tsk_stop()                 do { tsk_this->id = ID_RIP; return;                                             } while(0)
//      tsk_kill                        stop the task (tsk); it will no longer be executed
#define tsk_kill(tsk)              do { (tsk)->id = ID_RIP; if (tsk_self(tsk)) return;                             } while(0)
//      tsk_yield                       pass control to the next ready task
#define tsk_yield()                do { TSK_YIELD(true);                                                           } while(0)
//      tsk_flip                        restart the current task with function (fun)
#define tsk_flip(fun)              do { tsk_this->function = (fun); tsk_again();                                   } while(0)
//      tsk_sleepFor                    delay execution of current task for given duration of time (dly)
#define tsk_sleepFor(dly)          do { tmr_waitFor(&tsk_this->tmr, dly);                                          } while(0)
//      tsk_sleepNext                   delay execution of current task for given duration of time (dly) from the end of the previous countdown
#define tsk_sleepNext(dly)         do { tmr_waitNext(&tsk_this->tmr, dly);                                         } while(0)
//      tsk_sleepUntil                  delay execution of current task until given timepoint (tim)
#define tsk_sleepUntil(tim)        do { tmr_waitUntil(&tsk_this->tmr, tim);                                        } while(0)
//      tsk_sleep                       delay indefinitely execution of current task
#define tsk_sleep()                do { tsk_sleepFor(INFINITE);                                                    } while(0)
//      tsk_delay                       delay execution of current task for given duration of time (dly)
#define tsk_delay(dly)             do { tsk_sleepFor(dly);                                                         } while(0)
//      tsk_suspend                     suspend execution of the ready task (tsk)
#define tsk_suspend(tsk)           do { if ((tsk)->id == ID_RDY) { (tsk)->id = ID_DLY; TSK_YIELD(tsk_self(tsk)); } } while(0)
//      tsk_resume                      resume execution of the suspended task (tsk)
#define tsk_resume(tsk)            do { if ((tsk)->id == ID_DLY) { (tsk)->id = ID_RDY; }                           } while(0)

/* Timer ==================================================================== */

typedef cnt_t tmr_t;

/* -------------------------------------------------------------------------- */
#define OS_TMR(tmr)                     tmr_t tmr[] = { 0 }
/* -------------------------------------------------------------------------- */
//      tmr_start                       start the timer (tmr)
#define tmr_start(tmr)             do { *(tmr) = sys_time();                                                       } while(0)
//      tmr_expiredFor                  check if the timer (tmr) finishes countdown for given duration of time (dly)
#define tmr_expiredFor(tmr, dly)      ( sys_time() - *(tmr) + 1 > (dly) )
//      tmr_expiredUntil                check if the timer (tmr) finishes countdown until given timepoint (tim)
#define tmr_expiredUntil(tmr, tim)    ( tmr_expiredFor(tmr, (tim) - *(tmr)) )
//      tmr_waitFor                     wait until the timer (tmr) finishes countdown for given duration of time (dly)
#define tmr_waitFor(tmr, dly)      do { tmr_start(tmr); tsk_waitUntil(tmr_expiredFor(tmr, dly));  *(tmr) += (dly); } while(0)
//      tmr_waitNext                    wait until the timer (tmr) finishes countdown for given duration of time (dly) from the end of the previous countdown
#define tmr_waitNext(tmr, dly)     do {                 tsk_waitUntil(tmr_expiredFor(tmr, dly));  *(tmr) += (dly); } while(0)
//      tmr_waitUntil                   wait until the timer (tmr) finishes countdown until given timepoint (tim)
#define tmr_waitUntil(tmr, tim)    do { tmr_start(tmr); tsk_waitUntil(tmr_expiredUntil(tmr, tim)); *(tmr) = (tim); } while(0)

/* Binary semaphore ========================================================= */

typedef uint_fast8_t sem_t;

/* -------------------------------------------------------------------------- */
#define OS_SEM(sem, ini)                sem_t sem[] = { ini }
/* -------------------------------------------------------------------------- */
//      sem_wait                        wait for the semaphore (sem)
#define sem_wait(sem)              do { tsk_waitUntil(*(sem)); *(sem) = 0; } while(0)
//      sem_give                        release the semaphore (sem)
#define sem_give(sem)              do { *(sem) = 1;                        } while(0)

/* Protected signal ========================================================= */

typedef uint_fast8_t sig_t;

/* -------------------------------------------------------------------------- */
#define OS_SIG(sig)                     sig_t sig[] = { 0 }
/* -------------------------------------------------------------------------- */
//      sig_wait                        wait for the signal (sig)
#define sig_wait(sig)              do { tsk_waitUntil(*(sig)); } while(0)
//      sig_give                        release the signal (sig)
#define sig_give(sig)              do { *(sig) = 1;            } while(0)
//      sig_clear                       reset the signal (sig)
#define sig_clear(sig)             do { *(sig) = 0;            } while(0)

/* Event ==================================================================== */

typedef uintptr_t evt_t;

/* -------------------------------------------------------------------------- */
#define OS_EVT(evt)                     evt_t evt[] = { 0 }
/* -------------------------------------------------------------------------- */
//      evt_wait                        wait for a the new value of the event (evt)
#define evt_wait(evt)              do { *(evt) = 0; tsk_waitUntil(*(evt)); } while(0)
//      evt_take                        get a value of the event (evt)
#define evt_take(evt)                 ( *(evt) )
//      evt_give                        set a new value (val) of the event (evt)
#define evt_give(evt, val)         do { *(evt) = (val);                    } while(0)

/* Mutex ==================================================================== */

typedef tsk_t *mtx_t;

/* -------------------------------------------------------------------------- */
#define OS_MTX(mtx)                     mtx_t mtx[] = { 0 }
/* -------------------------------------------------------------------------- */
//      mtx_wait                        wait for the mutex (mtx)
#define mtx_wait(mtx)              do { tsk_waitWhile(*(mtx)); *(mtx) = tsk_this; } while(0)
//      mtx_give                        release previously owned mutex (mtx)
#define mtx_give(mtx)              do { if (*(mtx) == tsk_this) *(mtx) = 0;       } while(0)

/* -------------------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

#endif//__DUDOS
