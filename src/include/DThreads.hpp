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

class DThreads {
    private:
        static Thread *running;
        static std::list<Thread *> threads;
        static bool initialized;
    public:
        static void Init();
        static void Create(std::string name, std::function<void()> func);
        static void Yield();

};

#endif