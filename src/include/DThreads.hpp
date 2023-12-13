#ifndef DTHREADS_HPP
#define DTHREADS_HPP

/*
    public
    dthread_create() - Creates one user level thread.
    dthread_exit() - Exits and destroys a user level thread.
    dthread_wait() - Allows the parent to wait for all children to finish.

    private
    schedule() - Runs the scheduler.

*/

#include <string>
#include <functional>
#include <list>

#include "Thread.hpp"
#include "Synch.hpp"

class DThreads {
    private:
        static Thread *running;
        static Thread *idle;
        static std::list<Thread *> threads;
        static std::list<Thread *> ready;
        static bool initialized;

        static bool interruptsEnabled;

        static void DisableInterrupts();
        static void EnableInterrupts();

        static void Schedule();
        static void AddToReady(Thread *t);
        static bool SetInterrupts(bool enable);
        static void IdleThread();
    public:
        static void Init();
        static tid_t Create(std::string name, std::function<void()> func);
        static void Yield();
        static void Join(tid_t tid);
        
        static void CheckTimer();
        static Locks *CreateLock();

    friend class Locks;
    friend class Thread;
};

#endif