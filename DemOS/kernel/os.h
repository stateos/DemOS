/******************************************************************************

    @file    DemOS: os.h
    @author  Rajmund Szymanski
    @date    10.03.2020
    @brief   This file provides set of functions for DemOS.

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

#ifndef __DEMOS

#define __DEMOS_MAJOR        0
#define __DEMOS_MINOR        6
#define __DEMOS_BUILD        0

#define __DEMOS        ((((__DEMOS_MAJOR)&0xFFUL)<<24)|(((__DEMOS_MINOR)&0xFFUL)<<16)|((__DEMOS_BUILD)&0xFFFFUL))

#define __DEMOS__           "DemOS v" STRINGIZE(__DEMOS_MAJOR) "." STRINGIZE(__DEMOS_MINOR) "." STRINGIZE(__DEMOS_BUILD)

#define STRINGIZE(n) STRINGIZE_HELPER(n)
#define STRINGIZE_HELPER(n) #n

/* -------------------------------------------------------------------------- */

#include <stddef.h>
#include <stdint.h>
#include <limits.h>
#include <stdbool.h>
#include "osport.h"

#ifdef __cplusplus
extern "C" {
#endif

/* System =================================================================== */

#ifndef OS_FREQUENCY
#define OS_FREQUENCY  1000
#endif

#if    (OS_FREQUENCY)/1000000 > 0
#define USEC       (cnt_t)((OS_FREQUENCY)/1000000) // time multiplier given in microseconds
#endif
#if    (OS_FREQUENCY)/1000 > 0
#define MSEC       (cnt_t)((OS_FREQUENCY)/1000)    // time multiplier given in milliseconds
#endif
#define SEC        ((cnt_t)(OS_FREQUENCY))         // time multiplier given in seconds
#define MIN        ((cnt_t)(OS_FREQUENCY)*60)      // time multiplier given in minutes

#ifndef INFINITE
#define INFINITE   ((cnt_t)0-1)                    // time value for infinite wait
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
#define TSK_STATE(tag)                  sys_current->state = && TSK_LABEL(tag)
#define TSK_BEGIN()                     if (sys_current->state) goto *sys_current->state
#define TSK_END()

#else//!USE_GOTO

/* Task ( switch / case ) =================================================== */

typedef intptr_t tag_t;

#define TSK_LABEL(tag)                  /* falls through */ case tag
#define TSK_STATE(tag)                  sys_current->state = tag
#define TSK_BEGIN()                     switch (sys_current->state) { case 0:
#define TSK_END()                       }

#endif//USE_GOTO

/* Task ===================================================================== */
// definition of task function
typedef void fun_t( void );
// definition of task state
typedef enum   __tid { ID_RIP = 0, ID_RDY, ID_DLY } tid_t;
// definition of task structure
typedef struct __tsk { cnt_t tmr; tid_t id; tag_t state; fun_t *function; struct __tsk *next; } tsk_t;

// task initializer
#define TSK_INIT(fun)                   { 0, ID_RIP, 0, fun, NULL }

extern
tsk_t * sys_current;                 // system variable - current task
void    tsk_start( tsk_t *tsk );     // system function - make task ready to execute

/* -------------------------------------------------------------------------- */
// for internal use; start the system scheduler
#define SYS_START()                do { if ((sys_current = sys_current->next)->id == ID_RDY) sys_current->function(); } while(1)

/* -------------------------------------------------------------------------- */
// initialize the system timer and start the system scheduler
#define sys_start()                do { sys_init(); SYS_START(); } while(0)

/* -------------------------------------------------------------------------- */
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
#define TSK_WHILE(cnd)                  TSK_STATE(__LINE__); TSK_LABEL(__LINE__): if (cnd) return; (void)0
// for internal use; pass control to the next ready task if the condition (cnd) is true
#define TSK_YIELD(cnd)                  TSK_STATE(__LINE__); if (cnd) return; TSK_LABEL(__LINE__): (void)0
/* -------------------------------------------------------------------------- */
// return the current task
#define tsk_this()                    ( sys_current )
// alias
#define cur_task()                    ( sys_current )
// check whether the task (tsk) is the current task
#define tsk_self(tsk)                 ( sys_current == (tsk) )
// necessary prologue of the task
#define tsk_begin()                     TSK_BEGIN(); do {                                                          } while(0)
// necessary epilogue of the task
#define tsk_end()                       TSK_END(); do { tsk_again();                                               } while(0)
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
#define tsk_again()                do { sys_current->state = 0; return;                                            } while(0)
// stop the current task; it will no longer be executed
#define tsk_stop()                 do { sys_current->id = ID_RIP; return;                                          } while(0)
// stop the current task; it will no longer be executed
#define tsk_exit()                 do { sys_current->id = ID_RIP; return;                                          } while(0)
// stop the task (tsk); it will no longer be executed
#define tsk_kill(tsk)              do { (tsk)->id = ID_RIP; if (tsk_self(tsk)) return;                             } while(0)
// stop the current task; it will no longer be executed
#define cur_kill()                 do { sys_current->id = ID_RIP; return;                                          } while(0)
// restart the task (tsk) from the initial state
#define tsk_restart(tsk)           do { if (tsk_self(tsk)) tsk_again(); tsk_kill(tsk); tsk_start(tsk);             } while(0)
// restart the current task from the initial state
#define cur_restart()              do { tsk_again();                                                               } while(0)
// restart the task (tsk) with function (fun)
#define tsk_restartFrom(tsk, fun)  do { if (tsk_self(tsk)) tsk_flip(fun); tsk_kill(tsk); tsk_startFrom(tsk, fun);  } while(0)
// restart the current task with function (fun)
#define cur_restartFrom(fun)       do { tsk_flip(fun);                                                             } while(0)
// pass control to the next ready task
#define tsk_yield()                do { TSK_YIELD(true);                                                           } while(0)
// restart the current task with function (fun)
#define tsk_flip(fun)              do { sys_current->function = (fun); tsk_again();                                } while(0)
// delay execution of current task for given duration of time (dly)
#define tsk_sleepFor(dly)          do { tmr_waitFor(&sys_current->tmr, dly);                                       } while(0)
// delay execution of current task for given duration of time (dly) from the end of the previous countdown
#define tsk_sleepNext(dly)         do { tmr_waitNext(&sys_current->tmr, dly);                                      } while(0)
// delay execution of current task until given timepoint (tim)
#define tsk_sleepUntil(tim)        do { tmr_waitUntil(&sys_current->tmr, tim);                                     } while(0)
// delay indefinitely execution of current task
#define tsk_sleep()                do { tsk_sleepFor(INFINITE);                                                    } while(0)
// delay execution of current task for given duration of time (dly)
#define tsk_delay(dly)             do { tsk_sleepFor(dly);                                                         } while(0)
// suspend execution of the ready task (tsk)
#define tsk_suspend(tsk)           do { if ((tsk)->id == ID_RDY) { (tsk)->id = ID_DLY; TSK_YIELD(tsk_self(tsk)); } } while(0)
// suspend execution of the current task
#define cur_suspend()              do { sys_current->id = ID_DLY; TSK_YIELD(true);                                 } while(0)
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
// start/restart the timer (tmr)
#define tmr_start(tmr)             do { *(tmr) = sys_time();                                                       } while(0)
// start/restart the timer (tmr) and wait until the timer (tmr) finishes countdown for given duration of time (dly)
#define tmr_waitFor(tmr, dly)      do { tmr_start(tmr); tsk_waitUntil(tmr_expiredFor(tmr, dly)); *(tmr) += (dly);  } while(0)
// wait until the timer (tmr) finishes countdown for given duration of time (dly) from the end of the previous countdown
#define tmr_waitNext(tmr, dly)     do {                 tsk_waitUntil(tmr_expiredFor(tmr, dly)); *(tmr) += (dly);  } while(0)
// start/restart the timer (tmr) and wait until the timer (tmr) finishes countdown until given timepoint (tim)
#define tmr_waitUntil(tmr, tim)    do { tmr_start(tmr); tsk_waitUntil(tmr_expiredUntil(tmr, tim)); *(tmr) = (tim); } while(0)

/* Mutex ==================================================================== */
// definition of mutex
typedef tsk_t *mtx_t;

/* -------------------------------------------------------------------------- */
// define and initialize the mutex (mtx)
#define OS_MTX(mtx)                     mtx_t mtx[] = { NULL }
/* -------------------------------------------------------------------------- */
// try to lock the mutex (mtx); return true if the mutex has been successfully locked
#define mtx_take(mtx)                 ( *(mtx) ? false : ((*(mtx) = sys_current), true) )
// alias
#define mtx_tryLock(mtx)                mtx_take(mtx)
// wait for the released mutex (mtx) and lock it
#define mtx_wait(mtx)                   tsk_waitUntil(mtx_take(mtx))
// alias
#define mtx_lock(mtx)                   mtx_wait(mtx)
// release previously owned mutex (mtx); return true if the mutex has been successfully released
#define mtx_give(mtx)                 ( tsk_self(*(mtx)) ? ((*(mtx) = NULL), true) : false )
// alias
#define mtx_unlock(mtx)                 mtx_give(mtx)

/* Binary semaphore ========================================================= */
// definition of binary semaphore
typedef uint_fast8_t sem_t;

/* -------------------------------------------------------------------------- */
// define and initialize the binary semaphore (sem) with initial value (ini)
#define OS_SEM(sem, ini)                sem_t sem[] = { ini }
/* -------------------------------------------------------------------------- */
// return true if the semaphore (sem) is released
#define sem_given(sem)                ( *(sem) != 0 )
// try to lock the semaphore (sem); return true if the semaphore has been successfully locked
#define sem_take(sem)                 ( sem_given(sem) ? ((*(sem) = 0), true) : false )
// alias
#define sem_tryWait(sem)                sem_take(sem)
// wait for the released semaphore (sem) and lock it
#define sem_wait(sem)                   tsk_waitUntil(sem_take(sem))
// return true if the semaphore (sem) is locked
#define sem_taken(sem)                ( *(sem) == 0 )
// try to release the semaphore (sem); return true if the semaphore has been successfully released
#define sem_give(sem)                 ( sem_taken(sem) ? ((*(sem) = 1), true) : false )
// alias
#define sem_post(sem)                   sem_post(sem)
// wait for the locked semaphore (sem) and release it
#define sem_send(sem)                   tsk_waitUntil(sem_give(sem))

/* Protected signal ========================================================= */
// definition of protected signal
typedef unsigned sig_t;

#define SIG_LIMIT     (sizeof(unsigned) * CHAR_BIT)

#define SIGSET(signo) (((signo) < SIG_LIMIT) ? 1U << (signo) : 0U) // signal mask from the given signal number
#define sigAll        (0U-1)                                       // signal mask for all signals

/* -------------------------------------------------------------------------- */
// define and initialize the protected signal (sig)
#define OS_SIG(sig)                     sig_t sig[] = { 0 }
/* -------------------------------------------------------------------------- */
// return true if the signal number (num) of signal object (sig) is set
#define sig_take(sig, sigset)         ( (*(sig) & (sigset)) != 0 )
// alias
#define sig_tryWait(sig, sigset)        sig_take(sig, sigset)
// wait for the signal (sig) to be set
#define sig_wait(sig, sigset)           tsk_waitUntil(sig_take(sig, sigset))
// try to reset the signal (sig); return true if the signal has been successfully reset
#define sig_clear(sig, signo)         ( sig_take(sig, SIGSET(signo)) ? ((*(sig) &= ~SIGSET(signo)), true) : false )
// try to set the signal (sig); return true if the signal has been successfully set
#define sig_give(sig, signo)          ( sig_take(sig, SIGSET(signo)) ? false : ((*(sig) |= SIGSET(signo)), true) )
// alias
#define sig_set(sig, signo)             sig_give(sig, signo)

/* Event ==================================================================== */
// definition of event
typedef uintptr_t evt_t;

/* -------------------------------------------------------------------------- */
// define and initialize the event (evt)
#define OS_EVT(evt)                     evt_t evt[] = { 0 }
/* -------------------------------------------------------------------------- */
// get the event (evt) value
#define evt_take(evt)                 ( *(evt) )
// wait for a the new value of the event (evt)
#define evt_wait(evt)              do { *(evt) = 0; tsk_waitUntil(*(evt)); } while(0)
// set a new value (val) of the event (evt)
#define evt_give(evt, val)         do { *(evt) = (val);                    } while(0)

/* Job ====================================================================== */
// definition of job
typedef fun_t *job_t;

/* -------------------------------------------------------------------------- */
// define and initialize the job (job)
#define OS_JOB(job)                     job_t job[] = { NULL }
/* -------------------------------------------------------------------------- */
// try to take a function from the job (job); return true if the function has been successfully taken and executed
#define job_take(job)                 ( *(job) ? ((*(job))(), *(job) = NULL, true) : false )
// alias
#define job_tryWait(job)                job_take(job)
// wait for a the new job (job) function, execute it and release the job
#define job_wait(job)                   tsk_waitUntil(job_take(job))
// try to give the new function (fun) to the job (job); return true if the function has been successfully given
#define job_give(job, fun)            ( *(job) ? false : (*(job) = (fun), true) )
// wait for the released job (job) and give it a new function (fun)
#define job_send(job, fun)              tsk_waitUntil(job_give(job, fun))

/* Once flag ================================================================ */
// definition of once flag
typedef uint_fast8_t one_t;

/* -------------------------------------------------------------------------- */
// define and initialize the once flag (one)
#define OS_ONE(one)                     one_t one[] = { 0 }
/* -------------------------------------------------------------------------- */
// try to execute function (fun); if the once flag (one) has been previously set, the function is not executed
#define one_call(one, fun)         do { if (*(one) == 0) { (fun)(); *(one) = 1; } } while(0)

/* -------------------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

#endif//__DEMOS
