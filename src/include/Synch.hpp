#ifndef SYNCH_HPP
#define SYNCH_HPP

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

class Locks {
    private:
        bool acquired;
        std::list<Thread *> waiters;
        
    public:
        Locks();
        ~Locks();

        void Lock();
        void Unlock();
};

#endif