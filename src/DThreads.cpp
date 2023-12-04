#include <iostream>
#include <list>
#include <cassert>

#include <stdio.h>


#include "DThreads.hpp"
#include "Thread.hpp"

Thread *DThreads::running = nullptr;
std::list<Thread *> DThreads::threads;
std::list<Thread *> DThreads::ready;
bool DThreads::initialized = false;

void DThreads::Init() {
    /* Initialize by adding main thread to the list of threads. */
    DThreads::running = new Thread();
    DThreads::threads.push_back(running);

    /* Do not push to ready queue because it is currently running. */
    initialized = true;
}

/* Non-preemptive. Control returns to calling thread when done. */
void DThreads::Create(std::string name, std::function<void()> func) {
    assert(initialized);

    Thread *t = new Thread(name, func);
    DThreads::threads.push_back(t);
    DThreads::ready.push_back(t);
}

void DThreads::Schedule() {
    if (ready.size() == 0) {
        return;
    }

    /* Add ourselves to the ready list. */
    running->setStatus(Ready);
    ready.push_back(running);


    /* Pick a new thread to run. */
    Thread *t = ready.front();
    ready.pop_front();

    /* Mark it as running. */
    t->setStatus(Running);
    
    Thread *prev = running;
    Thread *next = t;

    /* Switch threads. */
    running = next;
    Thread::SwitchThreads(prev, next);

}

void DThreads::Yield() {
    assert(DThreads::initialized);

    if (running->getStatus() == Terminated) {
        assert(!"Termination not implemented yet!");
    }

    DThreads::Schedule();    
}