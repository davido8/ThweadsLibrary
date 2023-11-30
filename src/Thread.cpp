#include <string>
#include <iostream>
#include <functional>
#include <cassert>
#include <cerrno>

#include <sys/mman.h>
#include <unistd.h>

#include "Thread.hpp"

tid_t Thread::threadIDCount = 0;

// TODO: Not hardcode? I dunno. 8 MB should be fine for now.
size_t Thread::stackSize = 0x800000;
uintptr_t Thread::nextStack;

size_t PREALLOC = 4;
size_t PGSIZE;

Thread::Thread() {
    assert(id == 0);

    this->id = threadIDCount++;
    this->name = "main";
    this->started = true;

    /* Get the system page size. */
    PGSIZE = sysconf(_SC_PAGE_SIZE);
    long pgmask = PGSIZE - 1;

    /* Get the address of our current stack and round it down a page. */
    uintptr_t stackAddress;
    asm("mov %%rsp, %0" : "=r"(stackAddress));
    stackAddress &= ~pgmask;

    printf("Stack: %p\n", (void *) stackAddress);

    /* Allocate a page with no permissions to serve as guard page. */
    void *guardLocation = reinterpret_cast<void *>(stackAddress + stackSize);
    printf("Guard: %p\n", guardLocation);
    void *ptr = mmap(guardLocation, PGSIZE, PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (ptr != guardLocation || ptr == MAP_FAILED) {
        printf("Error: Could not map guard page at desired location. Got 0x%p instead of 0x%p\n", ptr, guardLocation);
        perror("");
        exit(EXIT_FAILURE);
    }
    Thread::nextStack = reinterpret_cast<uintptr_t>(guardLocation) + PGSIZE;

    printf(" Next: %p\n", (void *) nextStack);
}

Thread::Thread(std::string name, std::function<void()> func) {
    this->id = threadIDCount++;
    this->name = name;
    this->main = func;
    this->started = false;

    /* Give each thread its own stack so that they don't interleave. */
    void *stackLocation = reinterpret_cast<void *>(nextStack);
    void *ptr = mmap(
        stackLocation, 
        PREALLOC*PGSIZE, 
        PROT_WRITE | PROT_READ, 
        MAP_ANONYMOUS | MAP_PRIVATE, 
        -1, 
        0
    );
    if (ptr != stackLocation || ptr == MAP_FAILED) {
        printf("Error: Could not map stack at desired location. Got 0x%p instead of 0x%p\n", ptr, stackLocation);
        perror("");
        exit(EXIT_FAILURE);
    }

    /* Allocate a page with no permissions to serve as guard page. */
    void *guardLocation = reinterpret_cast<void *>(nextStack + stackSize);
    ptr = mmap(
        guardLocation, 
        PGSIZE, 
        PROT_NONE, 
        MAP_ANONYMOUS | MAP_PRIVATE, 
        -1, 
        0
    );
    if (ptr != guardLocation || ptr == MAP_FAILED) {
        printf("Error: Could not map guard page at desired location. Got 0x%p instead of 0x%p\n", ptr, guardLocation);
        perror("");
        exit(EXIT_FAILURE);
    }
    Thread::nextStack = reinterpret_cast<uintptr_t>(guardLocation) + PGSIZE;
}

void Thread::Start() {
    /* Main thread will have already been started. */
    if (!started) {
        main();
    }

    assert(!"Thread already started!\n");
}

void Thread::SaveContext() {

}