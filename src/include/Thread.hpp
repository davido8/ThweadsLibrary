#ifndef THREAD_HPP
#define THREAD_HPP

#include <string>
#include <functional>

using tid_t = int64_t;

enum ThreadStatus {
    Running,
    Ready,
    Terminated
};

struct Registers {
    // General-purpose int registers.
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rbp;
    uint64_t rsp;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;

    // Special registers.
    uint64_t rip;

    // TODO: Save the floating point registers.
};

class Thread {
    private:
        tid_t id;
        std::string name;
        std::function<void()> main;
        bool started;

        enum ThreadStatus status;
        struct Registers registers;

        /*------------------ Static Variables ------------------*/

        // Used to allocate new stacks for threads.
        static uintptr_t stackSize;
        static uintptr_t nextStack;

        static tid_t threadIDCount;

    public:
        /* Used to create new threads starting at func. */
        Thread(
            std::string name,
            std::function<void()> func
            // uintptr_t stackLocation
        );

        /* Used to initialize the main thread. */
        Thread();
        ~Thread();

        void Start();
        void SaveContext();
        void Terminate();

        enum ThreadStatus getStatus() { return status; }
};

#endif