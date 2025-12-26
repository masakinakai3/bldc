/**
 * @file ch_core.c
 * @brief ChibiOS POSIX implementation for PC build
 */

#define _GNU_SOURCE  /* For pthread_setname_np */
#include "ch.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/*===========================================================================*/
/* Global state                                                              */
/*===========================================================================*/

pthread_mutex_t _ch_sys_lock = PTHREAD_MUTEX_INITIALIZER;
volatile int _ch_lock_cnt = 0;

static pthread_key_t _current_thread_key;
static pthread_once_t _key_once = PTHREAD_ONCE_INIT;
static struct timespec _sys_start_time;

static void _create_thread_key(void) {
    pthread_key_create(&_current_thread_key, NULL);
}

/*===========================================================================*/
/* System functions                                                          */
/*===========================================================================*/

void chSysInit(void) {
    pthread_once(&_key_once, _create_thread_key);
    clock_gettime(CLOCK_MONOTONIC, &_sys_start_time);
    pthread_mutex_init(&_ch_sys_lock, NULL);
    _ch_lock_cnt = 0;
}

void chSysHalt(const char *reason) {
    fprintf(stderr, "SYSTEM HALT: %s\n", reason ? reason : "unknown");
    fflush(stderr);
    abort();
}

void chSysLock(void) {
    pthread_mutex_lock(&_ch_sys_lock);
    _ch_lock_cnt++;
}

void chSysUnlock(void) {
    _ch_lock_cnt--;
    pthread_mutex_unlock(&_ch_sys_lock);
}

void chSysLockFromISR(void) {
    chSysLock();
}

void chSysUnlockFromISR(void) {
    chSysUnlock();
}

/*===========================================================================*/
/* Time functions                                                            */
/*===========================================================================*/

systime_t chVTGetSystemTime(void) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    int64_t diff_ms = (now.tv_sec - _sys_start_time.tv_sec) * 1000 +
                      (now.tv_nsec - _sys_start_time.tv_nsec) / 1000000;
    return (systime_t)(diff_ms & 0xFFFFFFFF);
}

systime_t chVTGetSystemTimeX(void) {
    return chVTGetSystemTime();
}

bool chVTIsSystemTimeWithinX(systime_t start, systime_t end) {
    systime_t now = chVTGetSystemTimeX();
    return (now - start) < (end - start);
}

sysinterval_t chVTTimeElapsedSinceX(systime_t start) {
    return (sysinterval_t)(chVTGetSystemTimeX() - start);
}

systime_t chTimeAddX(systime_t systime, sysinterval_t interval) {
    return systime + interval;
}

/*===========================================================================*/
/* Delay functions                                                           */
/*===========================================================================*/

void chThdSleep(sysinterval_t time) {
    if (time == TIME_IMMEDIATE) return;
    if (time == TIME_INFINITE) {
        while (1) { sleep(3600); }
    }
    struct timespec ts;
    ts.tv_sec = time / 1000;
    ts.tv_nsec = (time % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

void chThdSleepMilliseconds(uint32_t msec) {
    chThdSleep(MS2ST(msec));
}

void chThdSleepMicroseconds(uint32_t usec) {
    struct timespec ts;
    ts.tv_sec = usec / 1000000;
    ts.tv_nsec = (usec % 1000000) * 1000;
    nanosleep(&ts, NULL);
}

void chThdSleepSeconds(uint32_t sec) {
    chThdSleep(S2ST(sec));
}

void chThdSleepUntil(systime_t time) {
    systime_t now = chVTGetSystemTime();
    if ((int32_t)(time - now) > 0) {
        chThdSleep(time - now);
    }
}

void chThdSleepUntilWindowed(systime_t prev, systime_t next) {
    (void)prev;
    chThdSleepUntil(next);
}

/*===========================================================================*/
/* Thread functions                                                          */
/*===========================================================================*/

static void* _thread_wrapper(void* arg) {
    thread_t *tp = (thread_t*)arg;
    pthread_setspecific(_current_thread_key, tp);
    tp->state = CH_STATE_CURRENT;
    void* result = tp->func(tp->arg);
    tp->state = CH_STATE_FINAL;
    return result;
}

thread_t *chThdCreateStatic(void *wsp, size_t size, tprio_t prio, tfunc_t pf, void *arg) {
    (void)size;
    (void)prio;
    
    thread_t *tp = (thread_t*)wsp;
    memset(tp, 0, sizeof(thread_t));
    tp->func = pf;
    tp->arg = arg;
    tp->state = CH_STATE_READY;
    tp->events = 0;
    pthread_mutex_init(&tp->evt_mutex, NULL);
    pthread_cond_init(&tp->evt_cond, NULL);
    
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    
    pthread_create(&tp->pthread, &attr, _thread_wrapper, tp);
    pthread_attr_destroy(&attr);
    
    return tp;
}

thread_t *chThdGetSelfX(void) {
    return (thread_t*)pthread_getspecific(_current_thread_key);
}

void chThdTerminate(thread_t *tp) {
    if (tp) {
        tp->state = CH_STATE_FINAL;
    }
}

bool chThdTerminatedX(thread_t *tp) {
    return tp ? (tp->state == CH_STATE_FINAL) : true;
}

bool chThdShouldTerminateX(void) {
    thread_t *tp = chThdGetSelfX();
    return tp ? (tp->state == CH_STATE_FINAL) : false;
}

void chThdSetPriority(tprio_t newprio) {
    (void)newprio;
    /* Priority not supported on POSIX without privileges */
}

tprio_t chThdGetPriorityX(void) {
    return NORMALPRIO;
}

msg_t chThdSuspendS(thread_reference_t *trp) {
    (void)trp;
    return MSG_OK;
}

msg_t chThdSuspendTimeoutS(thread_reference_t *trp, sysinterval_t timeout) {
    (void)trp;
    (void)timeout;
    return MSG_OK;
}

void chThdResumeI(thread_reference_t *trp, msg_t msg) {
    (void)trp;
    (void)msg;
}

void chThdResumeS(thread_reference_t *trp, msg_t msg) {
    (void)trp;
    (void)msg;
}

void chThdResume(thread_reference_t *trp, msg_t msg) {
    (void)trp;
    (void)msg;
}

msg_t chThdWait(thread_t *tp) {
    if (tp == NULL) return MSG_OK;
    void *retval;
    pthread_join(tp->pthread, &retval);
    return tp->exitcode;
}

void chThdExit(msg_t msg) {
    thread_t *tp = chThdGetSelfX();
    if (tp) {
        tp->exitcode = msg;
        tp->state = CH_STATE_FINAL;
    }
    pthread_exit((void*)(intptr_t)msg);
}

void chThdExitS(msg_t msg) {
    chThdExit(msg);
}

void chThdYield(void) {
    sched_yield();
}

void chRegSetThreadName(const char *name) {
    thread_t *tp = chThdGetSelfX();
    if (tp) {
        tp->name = name;
    }
#ifdef __linux__
    pthread_setname_np(pthread_self(), name);
#endif
}

/*===========================================================================*/
/* Mutex functions                                                           */
/*===========================================================================*/

void chMtxObjectInit(mutex_t *mp) {
    pthread_mutex_init(&mp->mutex, NULL);
    mp->owner = NULL;
    mp->cnt = 0;
}

void chMtxLock(mutex_t *mp) {
    pthread_mutex_lock(&mp->mutex);
    mp->owner = chThdGetSelfX();
    mp->cnt++;
}

void chMtxLockS(mutex_t *mp) {
    chMtxLock(mp);
}

bool chMtxTryLock(mutex_t *mp) {
    if (pthread_mutex_trylock(&mp->mutex) == 0) {
        mp->owner = chThdGetSelfX();
        mp->cnt++;
        return true;
    }
    return false;
}

bool chMtxTryLockS(mutex_t *mp) {
    return chMtxTryLock(mp);
}

void chMtxUnlock(mutex_t *mp) {
    mp->cnt--;
    if (mp->cnt == 0) {
        mp->owner = NULL;
    }
    pthread_mutex_unlock(&mp->mutex);
}

void chMtxUnlockS(mutex_t *mp) {
    chMtxUnlock(mp);
}

void chMtxUnlockAll(void) {
    /* Not implemented - would need to track all held mutexes */
}

bool chMtxQueueNotEmptyS(mutex_t *mp) {
    (void)mp;
    return false;
}

/*===========================================================================*/
/* Semaphore functions                                                       */
/*===========================================================================*/

void chSemObjectInit(semaphore_t *sp, cnt_t n) {
    sem_init(&sp->sem, 0, (unsigned int)n);
    sp->cnt = n;
}

void chSemReset(semaphore_t *sp, cnt_t n) {
    sem_destroy(&sp->sem);
    sem_init(&sp->sem, 0, (unsigned int)n);
    sp->cnt = n;
}

void chSemResetI(semaphore_t *sp, cnt_t n) {
    chSemReset(sp, n);
}

msg_t chSemWait(semaphore_t *sp) {
    if (sem_wait(&sp->sem) == 0) {
        sp->cnt--;
        return MSG_OK;
    }
    return MSG_RESET;
}

msg_t chSemWaitS(semaphore_t *sp) {
    return chSemWait(sp);
}

msg_t chSemWaitTimeout(semaphore_t *sp, sysinterval_t timeout) {
    if (timeout == TIME_IMMEDIATE) {
        if (sem_trywait(&sp->sem) == 0) {
            sp->cnt--;
            return MSG_OK;
        }
        return MSG_TIMEOUT;
    }
    if (timeout == TIME_INFINITE) {
        return chSemWait(sp);
    }
    
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += timeout / 1000;
    ts.tv_nsec += (timeout % 1000) * 1000000;
    if (ts.tv_nsec >= 1000000000) {
        ts.tv_sec++;
        ts.tv_nsec -= 1000000000;
    }
    
    if (sem_timedwait(&sp->sem, &ts) == 0) {
        sp->cnt--;
        return MSG_OK;
    }
    return (errno == ETIMEDOUT) ? MSG_TIMEOUT : MSG_RESET;
}

msg_t chSemWaitTimeoutS(semaphore_t *sp, sysinterval_t timeout) {
    return chSemWaitTimeout(sp, timeout);
}

void chSemSignal(semaphore_t *sp) {
    sp->cnt++;
    sem_post(&sp->sem);
}

void chSemSignalI(semaphore_t *sp) {
    chSemSignal(sp);
}

void chSemAddCounterI(semaphore_t *sp, cnt_t n) {
    sp->cnt += n;
    for (cnt_t i = 0; i < n; i++) {
        sem_post(&sp->sem);
    }
}

cnt_t chSemGetCounterI(const semaphore_t *sp) {
    return sp->cnt;
}

/*===========================================================================*/
/* Event functions (minimal implementation)                                  */
/*===========================================================================*/

void chEvtObjectInit(event_source_t *esp) {
    esp->next = NULL;
    pthread_mutex_init(&esp->mutex, NULL);
    pthread_cond_init(&esp->cond, NULL);
}

void chEvtRegisterMask(event_source_t *esp, event_listener_t *elp, eventmask_t events) {
    (void)esp; (void)elp; (void)events;
}

void chEvtRegister(event_source_t *esp, event_listener_t *elp, eventmask_t event) {
    chEvtRegisterMask(esp, elp, EVENT_MASK(event));
}

void chEvtUnregister(event_source_t *esp, event_listener_t *elp) {
    (void)esp; (void)elp;
}

eventmask_t chEvtGetAndClearEventsI(eventmask_t events) {
    (void)events;
    return 0;
}

eventmask_t chEvtGetAndClearEvents(eventmask_t events) {
    return chEvtGetAndClearEventsI(events);
}

eventmask_t chEvtAddEvents(eventmask_t events) {
    (void)events;
    return events;
}

eventmask_t chEvtAddEventsI(eventmask_t events) {
    return chEvtAddEvents(events);
}

void chEvtBroadcast(event_source_t *esp) {
    pthread_mutex_lock(&esp->mutex);
    pthread_cond_broadcast(&esp->cond);
    pthread_mutex_unlock(&esp->mutex);
}

void chEvtBroadcastI(event_source_t *esp) {
    chEvtBroadcast(esp);
}

void chEvtBroadcastFlags(event_source_t *esp, eventflags_t flags) {
    (void)flags;
    chEvtBroadcast(esp);
}

void chEvtBroadcastFlagsI(event_source_t *esp, eventflags_t flags) {
    chEvtBroadcastFlags(esp, flags);
}

eventmask_t chEvtWaitOne(eventmask_t events) {
    (void)events;
    return 0;
}

eventmask_t chEvtWaitAny(eventmask_t events) {
    (void)events;
    return 0;
}

eventmask_t chEvtWaitAll(eventmask_t events) {
    (void)events;
    return events;
}

eventmask_t chEvtWaitOneTimeout(eventmask_t events, sysinterval_t timeout) {
    (void)events; (void)timeout;
    return 0;
}

eventmask_t chEvtWaitAnyTimeout(eventmask_t events, sysinterval_t timeout) {
    (void)events; (void)timeout;
    return 0;
}

eventmask_t chEvtWaitAllTimeout(eventmask_t events, sysinterval_t timeout) {
    (void)events; (void)timeout;
    return 0;
}

eventflags_t chEvtGetAndClearFlags(event_listener_t *elp) {
    eventflags_t flags = elp->flags;
    elp->flags = 0;
    return flags;
}

eventflags_t chEvtGetAndClearFlagsI(event_listener_t *elp) {
    return chEvtGetAndClearFlags(elp);
}

void chEvtDispatch(const evhandler_t *handlers, eventmask_t events) {
    (void)handlers; (void)events;
}

/*===========================================================================*/
/* Thread-directed event functions                                           */
/*===========================================================================*/

void chEvtSignal(thread_t *tp, eventmask_t events) {
    if (!tp) return;
    
    pthread_mutex_lock(&tp->evt_mutex);
    tp->events |= events;
    pthread_cond_broadcast(&tp->evt_cond);
    pthread_mutex_unlock(&tp->evt_mutex);
}

void chEvtSignalI(thread_t *tp, eventmask_t events) {
    /* In PC/POSIX environment, ISR context is simulated - same as normal */
    chEvtSignal(tp, events);
}

/*===========================================================================*/
/* Virtual timer functions (stub implementation)                             */
/*===========================================================================*/

void chVTObjectInit(virtual_timer_t *vtp) {
    memset(vtp, 0, sizeof(virtual_timer_t));
}

void chVTSet(virtual_timer_t *vtp, sysinterval_t delay, vtfunc_t vtfunc, void *par) {
    (void)vtp; (void)delay; (void)vtfunc; (void)par;
    /* Timer functionality not fully implemented for PC */
}

void chVTSetI(virtual_timer_t *vtp, sysinterval_t delay, vtfunc_t vtfunc, void *par) {
    chVTSet(vtp, delay, vtfunc, par);
}

void chVTReset(virtual_timer_t *vtp) {
    (void)vtp;
}

void chVTResetI(virtual_timer_t *vtp) {
    chVTReset(vtp);
}

bool chVTIsArmed(const virtual_timer_t *vtp) {
    return vtp->func != NULL;
}

bool chVTIsArmedI(const virtual_timer_t *vtp) {
    return chVTIsArmed(vtp);
}

void chVTDoSetI(virtual_timer_t *vtp, sysinterval_t delay, vtfunc_t vtfunc, void *par) {
    chVTSetI(vtp, delay, vtfunc, par);
}

void chVTDoResetI(virtual_timer_t *vtp) {
    chVTResetI(vtp);
}

/*===========================================================================*/
/* Memory pool functions (stub implementation)                               */
/*===========================================================================*/

void chPoolObjectInit(memory_pool_t *mp, size_t size, void *provider) {
    (void)provider;
    mp->object_size = size;
    mp->free_list = NULL;
    pthread_mutex_init(&mp->mutex, NULL);
}

void chPoolLoadArray(memory_pool_t *mp, void *p, size_t n) {
    (void)mp; (void)p; (void)n;
}

void *chPoolAlloc(memory_pool_t *mp) {
    return malloc(mp->object_size);
}

void *chPoolAllocI(memory_pool_t *mp) {
    return chPoolAlloc(mp);
}

void chPoolFree(memory_pool_t *mp, void *objp) {
    (void)mp;
    free(objp);
}

void chPoolFreeI(memory_pool_t *mp, void *objp) {
    chPoolFree(mp, objp);
}

/*===========================================================================*/
/* Mailbox functions (stub implementation)                                   */
/*===========================================================================*/

void chMBObjectInit(mailbox_t *mbp, msg_t *buf, size_t n) {
    mbp->buffer = buf;
    mbp->top = &buf[n];
    mbp->wrptr = buf;
    mbp->rdptr = buf;
    mbp->n = n;
    chSemObjectInit(&mbp->fullsem, (cnt_t)n);
    chSemObjectInit(&mbp->emptysem, 0);
    chMtxObjectInit(&mbp->mtx);
}

void chMBReset(mailbox_t *mbp) {
    mbp->wrptr = mbp->buffer;
    mbp->rdptr = mbp->buffer;
    chSemReset(&mbp->fullsem, (cnt_t)mbp->n);
    chSemReset(&mbp->emptysem, 0);
}

void chMBResetI(mailbox_t *mbp) {
    chMBReset(mbp);
}

msg_t chMBPostTimeout(mailbox_t *mbp, msg_t msg, sysinterval_t timeout) {
    if (chSemWaitTimeout(&mbp->fullsem, timeout) != MSG_OK) {
        return MSG_TIMEOUT;
    }
    chMtxLock(&mbp->mtx);
    *mbp->wrptr++ = msg;
    if (mbp->wrptr >= mbp->top) {
        mbp->wrptr = mbp->buffer;
    }
    chMtxUnlock(&mbp->mtx);
    chSemSignal(&mbp->emptysem);
    return MSG_OK;
}

msg_t chMBPostTimeoutS(mailbox_t *mbp, msg_t msg, sysinterval_t timeout) {
    return chMBPostTimeout(mbp, msg, timeout);
}

msg_t chMBPostI(mailbox_t *mbp, msg_t msg) {
    return chMBPostTimeout(mbp, msg, TIME_IMMEDIATE);
}

msg_t chMBPostAhead(mailbox_t *mbp, msg_t msg, sysinterval_t timeout) {
    (void)mbp; (void)msg; (void)timeout;
    return MSG_OK;
}

msg_t chMBPostAheadS(mailbox_t *mbp, msg_t msg, sysinterval_t timeout) {
    return chMBPostAhead(mbp, msg, timeout);
}

msg_t chMBPostAheadI(mailbox_t *mbp, msg_t msg) {
    return chMBPostAhead(mbp, msg, TIME_IMMEDIATE);
}

msg_t chMBFetchTimeout(mailbox_t *mbp, msg_t *msgp, sysinterval_t timeout) {
    if (chSemWaitTimeout(&mbp->emptysem, timeout) != MSG_OK) {
        return MSG_TIMEOUT;
    }
    chMtxLock(&mbp->mtx);
    *msgp = *mbp->rdptr++;
    if (mbp->rdptr >= mbp->top) {
        mbp->rdptr = mbp->buffer;
    }
    chMtxUnlock(&mbp->mtx);
    chSemSignal(&mbp->fullsem);
    return MSG_OK;
}

msg_t chMBFetchTimeoutS(mailbox_t *mbp, msg_t *msgp, sysinterval_t timeout) {
    return chMBFetchTimeout(mbp, msgp, timeout);
}

msg_t chMBFetchI(mailbox_t *mbp, msg_t *msgp) {
    return chMBFetchTimeout(mbp, msgp, TIME_IMMEDIATE);
}

size_t chMBGetFreeCountI(const mailbox_t *mbp) {
    return (size_t)chSemGetCounterI(&mbp->fullsem);
}

size_t chMBGetUsedCountI(const mailbox_t *mbp) {
    return (size_t)chSemGetCounterI(&mbp->emptysem);
}

msg_t chMBPeekI(const mailbox_t *mbp) {
    if (chSemGetCounterI(&mbp->emptysem) <= 0) {
        return (msg_t)0;
    }
    return *mbp->rdptr;
}
