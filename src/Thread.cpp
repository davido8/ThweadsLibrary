#include <string>
#include <iostream>
#include <functional>

#include <sys/mman.h>

#include "Thread.hpp"

tid_t Thread::threadIDCount = 0;

Thread::Thread(
    std::string name, 
    std::function<void()> func
    // uintptr_t stackLocation
) {

    this->id = threadIDCount++;
    this->name = name;
    this->main = func;

    // Give each thread its own stack so that they don't interleave.
    // void *ptr = mmap(stackLocation, 0, PROT_WRITE | PROT_READ, MAP_ANONYMOUS);
    // if (ptr != stackLocation || ptr == MAP_FAILED) {
    //     printf("Error: Could not map stack at desired location.\n");
    //     exit(EXIT_FAILURE);
    // }
    std::cout << "Created thread " << name << " with tid: " << this->id << ".\n";

}

void Thread::Start() {
    main();
}

void Thread::SaveContext() {

}