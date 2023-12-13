#include <string>
#include <iostream>
#include <functional>
#include <cassert>
#include <cerrno>
#include <cstring>

#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>

#include "Thread.hpp"
#include "DThreads.hpp"

tid_t Thread::threadIDCount = 0;

// TODO: Not hardcode? I dunno. 8 MB should be fine for now.
size_t Thread::stackSize = 0x800000;
uintptr_t Thread::nextStack;

size_t PREALLOC = 20;
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

    /* Allocate a page with no permissions to serve as guard page. */
    void *guardLocation = reinterpret_cast<void *>(stackAddress + stackSize);
    void *ptr = mmap(guardLocation, PGSIZE, PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (ptr == MAP_FAILED) {
        printf("Error: Could not map guard page at desired location. Got 0x%p instead of 0x%p\n", ptr, guardLocation);
        perror("");
        exit(EXIT_FAILURE);
    }
    guardLocation = ptr;
    Thread::nextStack = reinterpret_cast<uintptr_t>(guardLocation) + PGSIZE;

    this->status = Running;
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
    if (ptr == MAP_FAILED) {
        printf("Error: Could not map stack at desired location. Got 0x%p instead of 0x%p\n", ptr, stackLocation);
        perror("");
        exit(EXIT_FAILURE);
    }
    stackLocation = ptr;

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
    if (ptr == MAP_FAILED) {
        printf("Error: Could not map guard page at desired location. Got 0x%p instead of 0x%p\n", ptr, guardLocation);
        perror("");
        exit(EXIT_FAILURE);
    }
    guardLocation = ptr;
    Thread::nextStack = reinterpret_cast<uintptr_t>(guardLocation) + PGSIZE;

    this->status = Ready;
    this->joiner = nullptr;

    /* Set %rip to point to Start() function. */
    this->registers.rip = reinterpret_cast<uintptr_t>((void *) &Thread::Start);

    this->registers.rbp = reinterpret_cast<uintptr_t>(stackLocation) + PGSIZE*PREALLOC;
    this->registers.rsp = reinterpret_cast<uintptr_t>(stackLocation) + PGSIZE*PREALLOC;
    this->registers.rdi = (uint64_t) ((void *) this);
    this->pushFakeRegisters();
}

void Thread::pushFakeRegisters() {
    uint64_t rsp = this->registers.rsp;

    /* Copy our register file onto the stack. */
    memcpy((void *) (rsp - sizeof(this->registers)), &this->registers, sizeof(this->registers));

    /* Save rsp and rbp. */
    this->registers.rbp = rsp;
    this->registers.rsp = rsp - sizeof(this->registers);
}

void Thread::Start() {
    /* Main thread will have already been started. */
    if (!started) {
        DThreads::SetInterrupts(true);
        this->main();
        this->Terminate();
    }

    assert(!"Thread already started!\n");
}

void Thread::printSavedRegisters() {
    printf("%s registers:\n", this->name.c_str());
    
    /* General-purpose registers. */
    printf("\t%s: 0x%lx\n", "rax", this->registers.rax);
    printf("\t%s: 0x%lx\n", "rbx", this->registers.rbx);
    printf("\t%s: 0x%lx\n", "rcx", this->registers.rcx);
    printf("\t%s: 0x%lx\n", "rdx", this->registers.rdx);
    printf("\t%s: 0x%lx\n", "rbp", this->registers.rbp);
    printf("\t%s: 0x%lx\n", "rsp", this->registers.rsp);
    printf("\t%s: 0x%lx\n", "rsi", this->registers.rsi);
    printf("\t%s: 0x%lx\n", "rdi", this->registers.rdi);
    printf("\t%s: 0x%lx\n", " r8", this->registers.r8);
    printf("\t%s: 0x%lx\n", " r9", this->registers.r9);
    printf("\t%s: 0x%lx\n", "r10", this->registers.r10);
    printf("\t%s: 0x%lx\n", "r11", this->registers.r11);
    printf("\t%s: 0x%lx\n", "r12", this->registers.r12);
    printf("\t%s: 0x%lx\n", "r13", this->registers.r13);
    printf("\t%s: 0x%lx\n", "r14", this->registers.r14);
    printf("\t%s: 0x%lx\n", "r15", this->registers.r15);

    printf("\n");

    printf("\t%s: %p\n", "rip", (void *) this->registers.rip);
}

void Thread::SwitchThreads(Thread *prev, Thread *next) {
    /* prev is stored in %rdi, next is stored in %rsi. */

    if (prev) {
        /* Push all registers onto the stack backwards. */
        asm volatile (
                "sub $0x10, %%rsp\n\t" /* GCC not decrementing for rdi and rsi... */
                "push %%r15\n\t"
                "push %%r14\n\t"
                "push %%r13\n\t"
                "push %%r12\n\t"
                "push %%r11\n\t"
                "push %%r10\n\t"
                "push %%r9\n\t"
                "push %%r8\n\t"
                "push %%rdi\n\t"
                "push %%rsi\n\t"
                "push %%rsp\n\t"
                "push %%rbp\n\t"
                "push %%rdx\n\t"
                "push %%rcx\n\t"
                "push %%rbx\n\t"
                "push %%rax\n\t"       

                /* Make sure to save rsp, rbp, and rip to register file. */
                "mov %%rbp, %0\n\t" 
                "mov %%rsp, %1\n\t" 
                "lea (%%rip), %2\n\t" 

            :   "=r"(prev->registers.rbp),
                "=r"(prev->registers.rsp),
                "=r"(prev->registers.rip)
        );

        if (prev->justRestored) {
            prev->justRestored = false;
            asm volatile ("add $0x10, %rsp\n\t");
            return;
        }
        prev->justRestored = true;
    }

    /* Restore next's context. */
    asm volatile (
            /* First restore RSP to get to saved registers. */
            "mov %0, %%rsp\n\t"

            /* Now read in the registers. */
            "pop %%rax\n\t"
            "pop %%rbx\n\t"
            "pop %%rcx\n\t"
            "pop %%rdx\n\t"
            "pop %%rsi\n\t"
            "pop %%rsi\n\t"
            "pop %%rsi\n\t"
            "pop %%rdi\n\t"
            "pop %%r8\n\t"
            "pop %%r9\n\t"
            "pop %%r10\n\t"
            "pop %%r11\n\t"
            "pop %%r12\n\t"
            "pop %%r13\n\t"
            "pop %%r14\n\t"
            "pop %%r15\n\t"
        :   
        :   "r"(next->registers.rsp)
        :   "memory"
    );

    asm (
            "mov %0, %%rbp\n\t"
            "jmp *%1\n\t"
        :
        :   "r"(next->registers.rbp),
            "r"(next->registers.rip)
        :   "memory"
    );
}

void Thread::Terminate() {
    this->status = Terminated;
    
    if (this->joiner) {
        this->joiner->Unblock();
    }

    DThreads::Yield();
}

void Thread::Block() {
    this->status = Blocked;
    DThreads::Yield();
}

void Thread::Unblock() {
    assert(this->status == Blocked);
    this->status = Ready;
    DThreads::AddToReady(this);
}

void Thread::JoinOn(Thread *t) {
    this->joiner = t;
}

Thread::~Thread() {}