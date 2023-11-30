#include <iostream>
#include <list>
#include <cassert>

#include <stdio.h>

#include "DThreads.hpp"
#include "Thread.hpp"

Thread *DThreads::running = nullptr;
std::list<Thread *> DThreads::threads;
bool DThreads::initialized = false;

void DThreads::Init() {
    /* Initialize by adding main thread to the list of threads. */
    DThreads::running = new Thread();
    DThreads::threads.push_back(running);
    initialized = true;
}

void DThreads::Create(std::string name, std::function<void()> func) {
    assert(initialized);

    Thread *t = new Thread(name, func);
    DThreads::threads.push_back(t);
}

void DThreads::Yield() {
    assert(DThreads::initialized);

    running->SaveContext();
    
}