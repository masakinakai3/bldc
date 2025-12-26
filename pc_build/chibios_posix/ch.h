/**
 * @file ch.h
 * @brief ChibiOS POSIX wrapper for PC build
 * 
 * Provides POSIX-based implementations of ChibiOS primitives:
 * - Threads (pthread)
 * - Mutexes (pthread_mutex)
 * - Semaphores (sem_t)
 * - Events (condition variables)
 * - Virtual timers (timer_t)
 */

#ifndef _CH_H_
#define _CH_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/
/* ChibiOS version info                                                      */
/*===========================================================================*/

#define CH_KERNEL_VERSION       "3.0.5-pc"
#define CH_KERNEL_MAJOR         3
#define CH_KERNEL_MINOR         0
#define CH_KERNEL_PATCH         5

/*===========================================================================*/
/* Return codes and status                                                   */
/*===========================================================================*/

typedef int32_t msg_t;

#define MSG_OK              ((msg_t)0)
#define MSG_TIMEOUT         ((msg_t)-1)
#define MSG_RESET           ((msg_t)-2)

#define RDY_OK              MSG_OK
#define RDY_TIMEOUT         MSG_TIMEOUT
#define RDY_RESET           MSG_RESET

/*===========================================================================*/
/* Time types                                                                */
/*===========================================================================*/

typedef uint32_t systime_t;
typedef uint32_t sysinterval_t;
typedef int32_t  cnt_t;

/* Time conversion macros - assume 1000 ticks per second for simplicity */
#define CH_CFG_ST_FREQUENCY     1000

#define TIME_IMMEDIATE      ((sysinterval_t)0)
#define TIME_INFINITE       ((sysinterval_t)-1)

#define TIME_MS2I(msec)     ((sysinterval_t)(msec))
#define TIME_US2I(usec)     ((sysinterval_t)((usec) / 1000))
#define TIME_S2I(sec)       ((sysinterval_t)((sec) * 1000))

#define MS2ST(msec)         TIME_MS2I(msec)
#define US2ST(usec)         TIME_US2I(usec)
#define S2ST(sec)           TIME_S2I(sec)

#define ST2MS(interval)     ((uint32_t)(interval))
#define ST2US(interval)     ((uint32_t)((interval) * 1000))

/* Legacy macros */
#define OSAL_MS2ST(msec)    MS2ST(msec)
#define OSAL_US2ST(usec)    US2ST(usec)
#define OSAL_S2ST(sec)      S2ST(sec)

/*===========================================================================*/
/* Event types - forward declarations needed for thread_t                   */
/*===========================================================================*/

typedef uint32_t eventmask_t;
typedef uint32_t eventflags_t;

/*===========================================================================*/
/* Stack and port types (for utils_sys.c compatibility)                     */
/*===========================================================================*/

/* Stack alignment type - must be defined before thread_t */
typedef uint32_t stkalign_t;

/* Port interrupt context (PC stub - not used on PC) */
struct port_intctx {
    uint32_t dummy;
};

/* ARM intrinsics stubs */
#ifndef __get_PSP
#define __get_PSP() ((uint32_t)0)
#endif

/*===========================================================================*/
/* Thread types                                                              */
/*===========================================================================*/

typedef void* (*tfunc_t)(void* arg);

typedef enum {
    CH_STATE_READY = 0,
    CH_STATE_CURRENT,
    CH_STATE_SUSPENDED,
    CH_STATE_SLEEPING,
    CH_STATE_FINAL
} tstate_t;

typedef struct thread {
    pthread_t       pthread;
    const char     *name;
    tstate_t        state;
    msg_t           exitcode;
    void           *arg;
    tfunc_t         func;
    struct thread  *next;
    eventmask_t     events;        /* Pending events for chEvtSignal */
    pthread_mutex_t evt_mutex;     /* Mutex for event operations */
    pthread_cond_t  evt_cond;      /* Condition variable for event wait */
    stkalign_t     *p_stklimit;    /* Stack limit pointer (for utils_sys.c) */
} thread_t;

/* Thread reference for chThdWait */
typedef thread_t* thread_reference_t;

/* Thread working area - on PC just allocate heap, no static sizing needed */
#define THD_WORKING_AREA_SIZE(n)    (sizeof(thread_t) + (n))
#define THD_WORKING_AREA(name, n)   uint8_t name[THD_WORKING_AREA_SIZE(n)]
#define THD_FUNCTION(name, arg)     void* name(void* arg)

/* Priority levels (mapped to nice values or ignored on POSIX) */
typedef int tprio_t;

#define NORMALPRIO      128
#define LOWPRIO         1
#define HIGHPRIO        255
#define IDLEPRIO        0

/*===========================================================================*/
/* Mutex types                                                               */
/*===========================================================================*/

typedef struct {
    pthread_mutex_t mutex;
    thread_t       *owner;
    uint32_t        cnt;
} mutex_t;

#define _MUTEX_DATA(name)   { PTHREAD_MUTEX_INITIALIZER, NULL, 0 }
#define MUTEX_DECL(name)    mutex_t name = _MUTEX_DATA(name)

/*===========================================================================*/
/* Semaphore types                                                           */
/*===========================================================================*/

typedef struct {
    sem_t           sem;
    cnt_t           cnt;
} semaphore_t;

typedef semaphore_t binary_semaphore_t;
typedef semaphore_t counting_semaphore_t;

#define _SEMAPHORE_DATA(name, n)    { .cnt = (n) }
#define SEMAPHORE_DECL(name, n)     semaphore_t name = _SEMAPHORE_DATA(name, n)

/*===========================================================================*/
/* Event types - structures                                                  */
/*===========================================================================*/

typedef struct event_listener event_listener_t;
typedef struct event_source event_source_t;

struct event_listener {
    event_listener_t   *next;
    thread_t           *listener;
    eventmask_t         events;
    eventflags_t        flags;
    eventflags_t        wflags;
};

struct event_source {
    event_listener_t   *next;
    pthread_mutex_t     mutex;
    pthread_cond_t      cond;
};

#define _EVENTSOURCE_DATA(name) { NULL, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER }
#define EVENTSOURCE_DECL(name)  event_source_t name = _EVENTSOURCE_DATA(name)

/* Event masks */
#define ALL_EVENTS      ((eventmask_t)-1)
#define EVENT_MASK(n)   ((eventmask_t)1 << (n))

/*===========================================================================*/
/* Virtual timer types                                                       */
/*===========================================================================*/

typedef void (*vtfunc_t)(void *p);

typedef struct virtual_timer {
    struct virtual_timer *next;
    struct virtual_timer *prev;
    systime_t           delta;
    vtfunc_t            func;
    void               *par;
    timer_t             timer_id;
} virtual_timer_t;

#define _VT_DATA(name)      { NULL, NULL, 0, NULL, NULL }

/*===========================================================================*/
/* Memory pool types                                                         */
/*===========================================================================*/

typedef struct {
    void           *next;
    size_t          object_size;
    size_t          alignment;
    void           *free_list;
    pthread_mutex_t mutex;
} memory_pool_t;

typedef struct {
    memory_pool_t  *pool;
    void           *objects;
    void           *limit;
    void           *head;
    size_t          obj_size;
    size_t          obj_num;
} guarded_memory_pool_t;

/*===========================================================================*/
/* Mailbox types                                                             */
/*===========================================================================*/

typedef struct {
    msg_t          *buffer;
    msg_t          *top;
    msg_t          *wrptr;
    msg_t          *rdptr;
    size_t          n;
    semaphore_t     fullsem;
    semaphore_t     emptysem;
    mutex_t         mtx;
} mailbox_t;

/*===========================================================================*/
/* System lock state                                                         */
/*===========================================================================*/

extern pthread_mutex_t _ch_sys_lock;
extern volatile int _ch_lock_cnt;

/*===========================================================================*/
/* System functions                                                          */
/*===========================================================================*/

void chSysInit(void);
void chSysHalt(const char *reason);
void chSysLock(void);
void chSysUnlock(void);
void chSysLockFromISR(void);
void chSysUnlockFromISR(void);

static inline bool chSysIsCounterWithinX(systime_t cnt, systime_t start, systime_t end) {
    return (bool)((systime_t)(cnt - start) < (systime_t)(end - start));
}

/*===========================================================================*/
/* Time functions                                                            */
/*===========================================================================*/

systime_t chVTGetSystemTime(void);
systime_t chVTGetSystemTimeX(void);
bool chVTIsSystemTimeWithinX(systime_t start, systime_t end);
sysinterval_t chVTTimeElapsedSinceX(systime_t start);
systime_t chTimeAddX(systime_t systime, sysinterval_t interval);

/* Delay functions */
void chThdSleep(sysinterval_t time);
void chThdSleepMilliseconds(uint32_t msec);
void chThdSleepMicroseconds(uint32_t usec);
void chThdSleepSeconds(uint32_t sec);
void chThdSleepUntil(systime_t time);
void chThdSleepUntilWindowed(systime_t prev, systime_t next);

/*===========================================================================*/
/* Thread functions                                                          */
/*===========================================================================*/

void chThdTerminate(thread_t *tp);
bool chThdTerminatedX(thread_t *tp);
bool chThdShouldTerminateX(void);
thread_t *chThdGetSelfX(void);
void chThdSetPriority(tprio_t newprio);
tprio_t chThdGetPriorityX(void);
msg_t chThdSuspendS(thread_reference_t *trp);
msg_t chThdSuspendTimeoutS(thread_reference_t *trp, sysinterval_t timeout);
void chThdResumeI(thread_reference_t *trp, msg_t msg);
void chThdResumeS(thread_reference_t *trp, msg_t msg);
void chThdResume(thread_reference_t *trp, msg_t msg);
thread_t *chThdCreateStatic(void *wsp, size_t size, tprio_t prio, tfunc_t pf, void *arg);
msg_t chThdWait(thread_t *tp);
void chThdExit(msg_t msg);
void chThdExitS(msg_t msg);
void chThdYield(void);
void chRegSetThreadName(const char *name);

/*===========================================================================*/
/* Mutex functions                                                           */
/*===========================================================================*/

void chMtxObjectInit(mutex_t *mp);
void chMtxLock(mutex_t *mp);
void chMtxLockS(mutex_t *mp);
bool chMtxTryLock(mutex_t *mp);
bool chMtxTryLockS(mutex_t *mp);
void chMtxUnlock(mutex_t *mp);
void chMtxUnlockS(mutex_t *mp);
void chMtxUnlockAll(void);
bool chMtxQueueNotEmptyS(mutex_t *mp);

/*===========================================================================*/
/* Semaphore functions                                                       */
/*===========================================================================*/

void chSemObjectInit(semaphore_t *sp, cnt_t n);
void chSemReset(semaphore_t *sp, cnt_t n);
void chSemResetI(semaphore_t *sp, cnt_t n);
msg_t chSemWait(semaphore_t *sp);
msg_t chSemWaitS(semaphore_t *sp);
msg_t chSemWaitTimeout(semaphore_t *sp, sysinterval_t timeout);
msg_t chSemWaitTimeoutS(semaphore_t *sp, sysinterval_t timeout);
void chSemSignal(semaphore_t *sp);
void chSemSignalI(semaphore_t *sp);
void chSemAddCounterI(semaphore_t *sp, cnt_t n);
cnt_t chSemGetCounterI(const semaphore_t *sp);

/* Binary semaphore wrappers */
#define chBSemObjectInit(bsp, taken)    chSemObjectInit((bsp), (taken) ? 0 : 1)
#define chBSemWait(bsp)                 chSemWait(bsp)
#define chBSemWaitS(bsp)                chSemWaitS(bsp)
#define chBSemWaitTimeout(bsp, t)       chSemWaitTimeout((bsp), (t))
#define chBSemSignal(bsp)               chSemSignal(bsp)
#define chBSemSignalI(bsp)              chSemSignalI(bsp)
#define chBSemReset(bsp, taken)         chSemReset((bsp), (taken) ? 0 : 1)
#define chBSemResetI(bsp, taken)        chSemResetI((bsp), (taken) ? 0 : 1)
#define chBSemGetStateI(bsp)            ((chSemGetCounterI(bsp) > 0) ? false : true)

/*===========================================================================*/
/* Event functions                                                           */
/*===========================================================================*/

void chEvtObjectInit(event_source_t *esp);
void chEvtRegisterMask(event_source_t *esp, event_listener_t *elp, eventmask_t events);
void chEvtRegister(event_source_t *esp, event_listener_t *elp, eventmask_t event);
void chEvtUnregister(event_source_t *esp, event_listener_t *elp);
eventmask_t chEvtGetAndClearEventsI(eventmask_t events);
eventmask_t chEvtGetAndClearEvents(eventmask_t events);
eventmask_t chEvtAddEvents(eventmask_t events);
eventmask_t chEvtAddEventsI(eventmask_t events);
void chEvtBroadcast(event_source_t *esp);
void chEvtBroadcastI(event_source_t *esp);
void chEvtBroadcastFlags(event_source_t *esp, eventflags_t flags);
void chEvtBroadcastFlagsI(event_source_t *esp, eventflags_t flags);
eventmask_t chEvtWaitOne(eventmask_t events);
eventmask_t chEvtWaitAny(eventmask_t events);
eventmask_t chEvtWaitAll(eventmask_t events);
eventmask_t chEvtWaitOneTimeout(eventmask_t events, sysinterval_t timeout);
eventmask_t chEvtWaitAnyTimeout(eventmask_t events, sysinterval_t timeout);
eventmask_t chEvtWaitAllTimeout(eventmask_t events, sysinterval_t timeout);
eventflags_t chEvtGetAndClearFlags(event_listener_t *elp);
eventflags_t chEvtGetAndClearFlagsI(event_listener_t *elp);

/* Thread-directed events */
void chEvtSignal(thread_t *tp, eventmask_t events);
void chEvtSignalI(thread_t *tp, eventmask_t events);

/* Event dispatch */
typedef void (*evhandler_t)(eventmask_t events);
void chEvtDispatch(const evhandler_t *handlers, eventmask_t events);

/*===========================================================================*/
/* Virtual timer functions                                                   */
/*===========================================================================*/

void chVTObjectInit(virtual_timer_t *vtp);
void chVTSet(virtual_timer_t *vtp, sysinterval_t delay, vtfunc_t vtfunc, void *par);
void chVTSetI(virtual_timer_t *vtp, sysinterval_t delay, vtfunc_t vtfunc, void *par);
void chVTReset(virtual_timer_t *vtp);
void chVTResetI(virtual_timer_t *vtp);
bool chVTIsArmed(const virtual_timer_t *vtp);
bool chVTIsArmedI(const virtual_timer_t *vtp);
void chVTDoSetI(virtual_timer_t *vtp, sysinterval_t delay, vtfunc_t vtfunc, void *par);
void chVTDoResetI(virtual_timer_t *vtp);

/*===========================================================================*/
/* Memory pool functions                                                     */
/*===========================================================================*/

void chPoolObjectInit(memory_pool_t *mp, size_t size, void *provider);
void chPoolLoadArray(memory_pool_t *mp, void *p, size_t n);
void *chPoolAlloc(memory_pool_t *mp);
void *chPoolAllocI(memory_pool_t *mp);
void chPoolFree(memory_pool_t *mp, void *objp);
void chPoolFreeI(memory_pool_t *mp, void *objp);

/*===========================================================================*/
/* Mailbox functions                                                         */
/*===========================================================================*/

void chMBObjectInit(mailbox_t *mbp, msg_t *buf, size_t n);
void chMBReset(mailbox_t *mbp);
void chMBResetI(mailbox_t *mbp);
msg_t chMBPostTimeout(mailbox_t *mbp, msg_t msg, sysinterval_t timeout);
msg_t chMBPostTimeoutS(mailbox_t *mbp, msg_t msg, sysinterval_t timeout);
msg_t chMBPostI(mailbox_t *mbp, msg_t msg);
msg_t chMBPostAhead(mailbox_t *mbp, msg_t msg, sysinterval_t timeout);
msg_t chMBPostAheadS(mailbox_t *mbp, msg_t msg, sysinterval_t timeout);
msg_t chMBPostAheadI(mailbox_t *mbp, msg_t msg);
msg_t chMBFetchTimeout(mailbox_t *mbp, msg_t *msgp, sysinterval_t timeout);
msg_t chMBFetchTimeoutS(mailbox_t *mbp, msg_t *msgp, sysinterval_t timeout);
msg_t chMBFetchI(mailbox_t *mbp, msg_t *msgp);
size_t chMBGetFreeCountI(const mailbox_t *mbp);
size_t chMBGetUsedCountI(const mailbox_t *mbp);
msg_t chMBPeekI(const mailbox_t *mbp);

/*===========================================================================*/
/* Debug / Assert macros                                                     */
/*===========================================================================*/

#define chDbgAssert(c, r)       do { if (!(c)) chSysHalt(r); } while(0)
#define chDbgCheck(c)           chDbgAssert(c, "chDbgCheck failed")
#define osalDbgAssert(c, r)     chDbgAssert(c, r)
#define osalDbgCheck(c)         chDbgCheck(c)

/*===========================================================================*/
/* OSAL layer compatibility                                                  */
/*===========================================================================*/

/* OSAL types */
typedef mutex_t osal_mutex_t;

/* System */
#define osalSysLock()           chSysLock()
#define osalSysUnlock()         chSysUnlock()
#define osalSysLockFromISR()    chSysLockFromISR()
#define osalSysUnlockFromISR()  chSysUnlockFromISR()
#define osalSysHalt(reason)     chSysHalt(reason)

/* Time */
#define osalOsGetSystemTimeX()  chVTGetSystemTimeX()
#define osalThreadSleep(t)      chThdSleep(t)
#define osalThreadSleepMilliseconds(m) chThdSleepMilliseconds(m)
#define osalThreadSleepMicroseconds(u) chThdSleepMicroseconds(u)

/* Mutexes */
#define osalMutexLock(mp)       chMtxLock(mp)
#define osalMutexUnlock(mp)     chMtxUnlock(mp)

/* Thread yield */
#define osalThreadYield()       chThdYield()

#ifdef __cplusplus
}
#endif

#endif /* _CH_H_ */
