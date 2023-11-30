#include <iostream>
#include <list>
#include <cassert>

#include <stdio.h>

#include "DThreads.hpp"
#include "Thread.hpp"

std::list<Thread *> DThreads::threads;
bool DThreads::initialized = false;

// TODO: Not hardcode? I dunno. 8 MB should be fine for now.
uintptr_t  DThreads::stackGap = 0x800000;
uintptr_t DThreads::lastStack = 0;

void DThreads::Init() {
    // Initialize by adding main thread to the list of threads.
    std::cout << "Thread subsystem initialized.\n";

    asm("mov %%rsp, %0" : "=r"(lastStack));
    lastStack = lastStack & (~0xFFFF);

    printf("Main's stack is at 0x%ld, next one will be at 0x%ld\n", lastStack, lastStack + stackGap);



    // TODO
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