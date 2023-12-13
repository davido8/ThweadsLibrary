#include <iostream>
#include <list>
#include <cassert>
#include <csignal>
#include <sys/types.h>
#include <unistd.h>

#include <stdio.h>
#include <sys/time.h>

#include "DThreads.hpp"
#include "Thread.hpp"

const suseconds_t TIMESLICE_MICRO = 100*1000;

Thread *DThreads::running = nullptr;
Thread *DThreads::idle = nullptr;
std::list<Thread *> DThreads::threads;
std::list<Thread *> DThreads::ready;
bool DThreads::initialized = false;
bool DThreads::interruptsEnabled = false;

sigset_t allsignals;
sigset_t oldsignals;

sig_atomic_t timerExpired = 0;
static void TimerInterruptHandler(int sig) {
    timerExpired = 1;
}

void DThreads::CheckTimer() {
    if (timerExpired == 1) {
        timerExpired = 0;
        DThreads::Yield();
    }
}

bool DThreads::SetInterrupts(bool enable) {
    bool old = interruptsEnabled;
    if (enable) {
        if (!interruptsEnabled) {
            EnableInterrupts();
        }
    }

    if (!enable) {
        if (interruptsEnabled) {
            DisableInterrupts();
        }
    }
    return old;
}
void DThreads::DisableInterrupts() {
    if (sigprocmask(SIG_BLOCK, &allsignals, &oldsignals) == -1) {
        perror("Error: ");
        exit(EXIT_FAILURE);
    }

    interruptsEnabled = false;
}

void DThreads::EnableInterrupts() {
    if (sigprocmask(SIG_SETMASK, &oldsignals, nullptr) == -1) {
        perror("Error: ");
        exit(EXIT_FAILURE);
    }

    interruptsEnabled = true;
}

void DThreads::IdleThread() {
    while (true) {
        bool old = DThreads::SetInterrupts(false);
        size_t blocked = 0;
        for (Thread *t: threads) {
            if (t->getStatus() == Blocked || t->getStatus() == Terminated) {
                blocked++;
            }
        }
        if (blocked == threads.size()) {
            printf("Idle: All threads are blocked!\n");
            exit(EXIT_FAILURE);
        }
        DThreads::SetInterrupts(old);

        DThreads::CheckTimer();
    }
}

void DThreads::Init() {
    /* Initialize by adding main thread to the list of threads. */
    DThreads::running = new Thread();
    DThreads::idle = new Thread("idle", IdleThread);

    DThreads::threads.push_back(running);

    /* Do not push to ready queue because it is currently running. */

    /* Start the preemptive scheduler. */

    struct sigaction action;
    action.sa_handler = TimerInterruptHandler;
    action.sa_flags = SA_RESTART | SA_NODEFER;
    if (sigaction(SIGALRM, &action, nullptr) == -1) {
        perror("Error: sigaction");
        exit(EXIT_FAILURE);
    }

    if (sigfillset(&allsignals) == -1) {
        perror("Error: ");
        exit(EXIT_FAILURE);
    }

    struct itimerval timer;
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = TIMESLICE_MICRO;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = TIMESLICE_MICRO;

    if (setitimer(ITIMER_REAL, &timer, nullptr) != 0) {
        initialized = false;
        perror("Error: setitimer()");
        exit(EXIT_FAILURE);
    }

    initialized = true;
    interruptsEnabled = true;
}

/* Non-preemptive. Control returns to calling thread when done. */
tid_t DThreads::Create(std::string name, std::function<void()> func) {
    assert(initialized);

    bool old = SetInterrupts(false);
    tid_t tid = -1;
    Thread *t = new Thread(name, func);
    if (t) {
        DThreads::threads.push_back(t);
        DThreads::ready.push_back(t);
        tid = t->getTID();
    }
    SetInterrupts(old);
    return tid;
}

void DThreads::Schedule() {
    bool old = SetInterrupts(false);

    /* Add ourselves to the ready list only if not terminated or blocked. */
    if (running->getStatus() == Running) {
        running->setStatus(Ready);
        ready.push_back(running);        
    }

    Thread *t = idle;

    /* Pick a new thread to run. */
    if (ready.size() != 0) {
        t = ready.front();
        ready.pop_front();
    }

    /* Mark it as running. */
    t->setStatus(Running);
    
    Thread *prev = running;
    Thread *next = t;

    /* Handle termination. */
    if (prev->getStatus() == Terminated) {
        prev = nullptr;
        threads.remove(prev);
        delete prev;
    }

    /* Switch threads. */
    running = next;
    Thread::SwitchThreads(prev, next);

    SetInterrupts(old);
}

void DThreads::Yield() {
    assert(DThreads::initialized);
    DThreads::Schedule();
}

/*
    Waits for a thread to terminate. Each thread can only be waited on once.
*/
void DThreads::Join(tid_t tid) {    

    bool old = SetInterrupts(false);
    Thread *t = nullptr;
    for (Thread *thd: threads) {
        if (thd->getTID() == tid) {
            t = thd;
        }
    }

    if (!t || t->getStatus() == Terminated) {
        return;
    }

    /* Set ourself to be notified when terminated. */
    t->JoinOn(running);
    running->Block();

    /* TODO: Race condition if multiple threads wait before terminated. */
    /* TODO: Release the threads resources like stack pages. */
    threads.remove(t);
    delete t;
    SetInterrupts(old);
}

void DThreads::AddToReady(Thread *t) {
    assert(t->getStatus() == Ready);

    bool old = SetInterrupts(false);
    /* Make sure this is a thread. */
    bool found = false;
    for (Thread *thd: threads) {
        if (t->getTID() == thd->getTID()) {
            found = true;
            break;
        }
    }

    if (!found) {
        assert(!"Adding fake thread!");
    }

    /* Make sure it is not in the ready list already. */
    bool alreadyInside = false;
    for (Thread *thd: ready) {
        if (t->getTID() == thd->getTID()) {
            alreadyInside = true;
            break;
        }
    }

    if (alreadyInside) {
        assert(!"Thread in ready list already!");
    }

    ready.push_back(t);

    SetInterrupts(old);
}

Locks *DThreads::CreateLock() {
    return new Locks();
}
