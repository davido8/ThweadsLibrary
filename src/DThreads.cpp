#include <iostream>
#include <list>
#include <cassert>

#include <stdio.h>

#include "DThreads.hpp"
#include "Thread.hpp"

std::list<Thread *> DThreads::threads;
bool DThreads::initialized = false;

void DThreads::Init() {
    /* Initialize by adding main thread to the list of threads. */
    DThreads::threads.push_back(new Thread());

    initialized = true;
}

void DThreads::Create(std::string name, std::function<void()> func) {
    assert(initialized);

    Thread *t = new Thread(name, func);
    DThreads::threads.push_back(t);
    t->Start(); 

}

void DThreads::Yield() {
    assert(DThreads::initialized);

    // TODO: Make sure we are in a thread context.
    std::cout << "Thread yield called.\n";
}