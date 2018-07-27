/******************************************************************************

    @file    DudOS: os.h
    @author  Rajmund Szymanski
    @date    27.07.2018
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

typedef void fun_t( void );

/* Public ------------------------------------------------------------------- */
void    sys_init ( void );           // port function - initialize the system timer
cnt_t   sys_time ( void );           // port function - get current value of the system timer (in milliseconds)
void    sys_start( void );           // system function - start the system scheduler
/* -------------------------------------------------------------------------- */
#define MSEC                  1      // time multiplier given in milliseconds
#define SEC                1000      // time multiplier given in seconds
#define MIN               60000      // time multiplier given in minutes
#ifndef INFINITE
#define INFINITE           ( ~0UL )  // time value for infinite wait
#endif

/* Task ( goto / label ) ==================================================== */

#ifdef  USE_GOTO

typedef struct __tsk { cnt_t start; cnt_t delay; void *state; fun_t *function; struct __tsk *next; } tsk_t;

/* Private ------------------------------------------------------------------ */
#define TSK_CONCATENATE(tag)            Line ## tag
#define TSK_LABEL(tag)                  TSK_CONCATENATE(tag)
#define TSK_STATE(tag)               && TSK_LABEL(tag)
#define TSK_BEGIN(tsk)                  if ((tsk)->state) goto *(tsk)->state
#define TSK_END()

/* Task ( switch / case ) =================================================== */

#else//!USE_GOTO

typedef struct __tsk { cnt_t start; cnt_t delay; intptr_t state; fun_t *function; struct __tsk *next; } tsk_t;

/* Private ------------------------------------------------------------------ */
#define TSK_LABEL(tag)                  /* falls through */ case tag
#define TSK_STATE(tag)                  tag
#define TSK_BEGIN(tsk)                  switch ((tsk)->state) { default:
#define TSK_END()                       }

#endif//USE_GOTO

#define OBJ_GET(obj)                    (*(obj)         )
#define OBJ_PUT(obj, val)               (*(obj)  = (val))
#define OBJ_CLR(obj)                    (*(obj)  = 0    )
#define OBJ_SET(obj)                    (*(obj)  = 1    )
#define OBJ_ADD(obj, val)               (*(obj) += (val))
#define OBJ_SUB(obj, val)               (*(obj) -= (val))
#define OBJ_INC(obj)                    (*(obj) += 1    )
#define OBJ_DEC(obj)                    (*(obj) -= 1    )
/* -------------------------------------------------------------------------- */
#define TSK_INIT(tsk)                   ((tsk)->state = 0)
#define TSK_GOTO(tsk, tag)              ((tsk)->state = (tag))
#define TSK_STOP(tsk)                   ((tsk)->function = 0)
#define TSK_START(tsk, fun)             ((tsk)->function = (fun))
#define TSK_WORK(tsk)                   ((tsk)->function != 0)
#define TSK_CALL(tsk)                   if (TSK_WORK(tsk)) (tsk)->function()
#define TSK_WHILE(tsk, cnd)             TSK_GOTO(tsk, TSK_STATE(__LINE__)); TSK_LABEL(__LINE__): if (cnd) return; TSK_INIT(tsk)
#define TSK_YIELD(tsk, cnd)             TSK_GOTO(tsk, TSK_STATE(__LINE__)); if (cnd) return; TSK_LABEL(__LINE__): TSK_INIT(tsk)

/* Public ------------------------------------------------------------------- */
#define  OS_TSK(tsk, fun)               tsk_t tsk[1] = { { 0, 0, 0, fun, 0 } }

#define  OS_TSK_DEF(tsk)                void tsk ## __fun( void ); \
                                        OS_TSK(tsk, tsk ## __fun); \
                                        void tsk ## __fun( void )

#define  OS_TSK_START(tsk)              void tsk ## __fun( void ); \
                                        OS_TSK(tsk, tsk ## __fun); \
           __attribute__((constructor)) void tsk ## __run( void ) { tsk_start(tsk); } \
                                        void tsk ## __fun( void )
/* -------------------------------------------------------------------------- */
bool    tsk_start( tsk_t *tsk );     // system function - add task to queue
tsk_t * tsk_this ( void );           // system function - return current task
/* -------------------------------------------------------------------------- */
//      tsk_begin                       necessary task prologue
#define tsk_begin()                     tsk_t * const Current = tsk_this(); TSK_BEGIN(Current); do {} while(0)
//      tsk_end                         necessary task epilogue
#define tsk_end()                       TSK_END();                                              do {} while(0)
//      tsk_waitWhile                   wait while the condition (cnd) is true
#define tsk_waitWhile(cnd)         do { TSK_WHILE(Current, cnd);                                    } while(0)
//      tsk_waitUntil                   wait while the condition (cnd) is false
#define tsk_waitUntil(cnd)         do { tsk_waitWhile(!(cnd));                                      } while(0)
//      tsk_join                        wait while the task (tsk) is working
#define tsk_join(tsk)              do { tsk_waitWhile(TSK_WORK(tsk));                               } while(0)
//      tsk_startFrom                   start new or restart previously stopped task (tsk) with function (fun)
#define tsk_startFrom(tsk, fun)    do { if (tsk_start(tsk) || !TSK_WORK(tsk)) TSK_START(tsk, fun);  } while(0)
//      tsk_exit                        restart the current task from the initial state
#define tsk_exit()                 do { return;                                                     } while(0)
//      tsk_stop                        stop the current task; it will no longer be executed
#define tsk_stop()                 do { TSK_STOP(Current); return;                                  } while(0)
//      tsk_kill                        stop the task (tsk); it will no longer be executed
#define tsk_kill(tsk)              do { TSK_STOP(tsk); if ((tsk) == Current) return; TSK_INIT(tsk); } while(0)
//      tsk_yield                       pass control to the next ready task
#define tsk_yield()                do { TSK_YIELD(Current, true);                                   } while(0)
//      tsk_flip                        restart the current task with function (fun)
#define tsk_flip(fun)              do { TSK_START(Current, fun); return;                            } while(0)
//      tsk_sleepFor                    delay execution of current task for given duration of time (dly)
#define tsk_sleepFor(dly)          do { tmr_waitFor(Current, dly);                                  } while(0)
//      tsk_sleepUntil                  delay execution of current task until given timepoint (tim)
#define tsk_sleepUntil(tim)        do { tmr_waitUntil(Current, tim);                                } while(0)
//      tsk_sleepAgain                  delay again execution of current task for previously given duration of time
#define tsk_sleepAgain(tsk)        do { tmr_waitAgain(Current);                                     } while(0)
//      tsk_sleep                       delay indefinitely execution of current task
#define tsk_sleep()                do { tsk_sleepFor(INFINITE);                                     } while(0)
//      tsk_delay                       delay execution of current task for given duration of time (dly)
#define tsk_delay(dly)             do { tsk_sleepFor(dly);                                          } while(0)
//      tsk_suspend                     suspend the current task
#define tsk_suspend()              do { tsk_sleep();                                                } while(0)
//      tsk_resume                      resume the task (tsk)
#define tsk_resume(tsk)            do { tmr_stop(tsk);                                              } while(0)

/* Timer ==================================================================== */

typedef struct __tmr { cnt_t start; cnt_t delay; } tmr_t;

/* Private ------------------------------------------------------------------ */
#define TMR_INIT(tmr, dly)              (tmr)->start = sys_time(); (tmr)->delay = (dly)
#define TMR_WORK(tmr)                   (sys_time() - (tmr)->start + 1 <= (tmr)->delay)

/* Public ------------------------------------------------------------------- */
#define  OS_TMR(tmr)                    tmr_t tmr[1] = { { 0, 0   } }
#define  OS_TMR_START(tmr, dly)         tmr_t tmr[1] = { { 0, dly } }
/* -------------------------------------------------------------------------- */
//      tmr_startFor                    start the timer (tmr) for given duration of time (dly)
#define tmr_startFor(tmr, dly)     do { TMR_INIT(tmr, dly);                      } while(0)
//      tmr_startUntil                  start the timer (tmr) until given timepoint (tim)
#define tmr_startUntil(tmr, tim)   do { TMR_INIT(tmr, (tim) - (tmr)->start);     } while(0)
//      tmr_startAgain                  restart the timer (tmr) for previously given duration of time
#define tmr_startAgain(tmr)        do { (tmr)->start += (tmr)->delay;            } while(0)
//      tmr_stop                        stop the timer (tmr)
#define tmr_stop(tmr)              do { TMR_INIT(tmr, 0);                        } while(0)
//      tmr_expired                     check if the timer (tmr) finishes countdown
#define tmr_expired(tmr)              ( !TMR_WORK(tmr) )
//      tmr_wait                        wait indefinitely until the timer (tmr) finishes countdown
#define tmr_wait(tmr)              do { tsk_waitUntil(tmr_expired(tmr));         } while(0)
//      tmr_waitFor                     wait until the timer (tmr) finishes countdown for given duration of time (dly)
#define tmr_waitFor(tmr, dly)      do { tmr_startFor(tmr, dly);   tmr_wait(tmr); } while(0)
//      tmr_waitUntil                   wait until the timer (tmr) finishes countdown until given timepoint (tim)
#define tmr_waitUntil(tmr, tim)    do { tmr_startUntil(tmr, tim); tmr_wait(tmr); } while(0)
//      tmr_waitAgain                   wait again until the timer (tmr) finishes countdown for previously given duration of time
#define tmr_waitAgain(tmr)         do { tmr_startAgain(tmr);      tmr_wait(tmr); } while(0)

/* Binary semaphore ========================================================= */

typedef uint_fast8_t sem_t;

/* Public ------------------------------------------------------------------- */
#define  OS_SEM(sem, ini)               sem_t sem[1] = { ini }
/* -------------------------------------------------------------------------- */
//      sem_wait                        wait for the semaphore (sem)
#define sem_wait(sem)              do { tsk_waitUntil(OBJ_GET(sem)); OBJ_CLR(sem); } while(0)
//      sem_give                        release the semaphore (sem)
#define sem_give(sem)              do { OBJ_SET(sem);                              } while(0)

/* Protected signal ========================================================= */

typedef uint_fast8_t sig_t;

/* Public ------------------------------------------------------------------- */
#define  OS_SIG(sig)                    sig_t sig[1] = { 0 }
/* -------------------------------------------------------------------------- */
//      sig_wait                        wait for the signal (sig)
#define sig_wait(sig)              do { tsk_waitUntil(OBJ_GET(sig)); } while(0)
//      sig_give                        release the signal (sig)
#define sig_give(sig)              do { OBJ_SET(sig);                } while(0)
//      sig_clear                       reset the signal (sig)
#define sig_clear(sig)             do { OBJ_CLR(sig);                } while(0)

/* Event ==================================================================== */

typedef uintptr_t evt_t;

/* Public ------------------------------------------------------------------- */
#define  OS_EVT(evt)                    evt_t evt[1] = { 0 }
/* -------------------------------------------------------------------------- */
//      evt_wait                        wait for a the new value of the event (evt)
#define evt_wait(evt)              do { OBJ_CLR(evt); tsk_waitUntil(OBJ_GET(evt)); } while(0)
//      evt_take                        get a value of the event (evt)
#define evt_take(evt)                 ( OBJ_GET(evt) )
//      evt_give                        set a new value (val) of the event (evt)
#define evt_give(evt, val)         do { OBJ_PUT(evt, val);                         } while(0)

/* Mutex ==================================================================== */

typedef tsk_t *mtx_t;

/* Public ------------------------------------------------------------------- */
#define  OS_MTX(mtx)                    mtx_t mtx[1] = { 0 }
/* -------------------------------------------------------------------------- */
//      mtx_wait                        wait for the mutex (mtx)
#define mtx_wait(mtx)              do { tsk_waitWhile(OBJ_GET(mtx)); OBJ_PUT(mtx, Current); } while(0)
//      mtx_give                        release previously owned mutex (mtx)
#define mtx_give(mtx)              do { if (OBJ_GET(mtx) == Current) OBJ_CLR(mtx);          } while(0)

/* -------------------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

#endif//__DUDOS
