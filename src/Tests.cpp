#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <string.h>
#include <cassert>
#include <stdint.h>
#include <string>
#include "DThreads.hpp"
#include <vector>
#include <unistd.h>

static void recurse() {
    int x;
    printf("Stack at: %p\n", &x);
    recurse();
}
/*
    Each thread reserves an amount of memory for its stack. At the end of this 
    memory is a page with no permissions so that if they overflow they crash
    instead of running into someone else's stack. This tests to ensure that it
    properly crashes.
*/
void testMainOverflow() {
    recurse();
}

void testThreadOverflow() {
    tid_t tid = DThreads::Create("thread 1", recurse);
    DThreads::Join(tid);
}

/* -------------------------------------------------------------------------- */

static void allocateStackVariables() {
    char buf[100];
    memset(buf, 0, sizeof(buf));
    strncpy(buf, "Pikmin", 7);
    printf("buf: %s\n", buf);
}

/*
    Tests that setting %rsp to the newly mmap'ed stack doesn't result in a
    segmentation fault. And when you don't mmap it, it does. Does so by calling
    a function which allocates a lot of stack variables.

    Note: Requires modifying source code to not mmap page.
*/
void stackMappedCorrectly() {
    tid_t tid = DThreads::Create("thread 1", allocateStackVariables);
    DThreads::Join(tid);
}

/* -------------------------------------------------------------------------- */

void SleepEnsured(time_t seconds, long nanoseconds) {
    struct timespec req, rem;
    req.tv_sec = seconds; 
    req.tv_nsec = nanoseconds;

    while (nanosleep(&req, &rem) == -1) {
        req = rem;
        DThreads::CheckTimer();
    }
}

static void printMessageYieldLoop(std::string msg) {
    for (int i = 0; i < 5; i++) {
        printf("%s\n", msg.c_str());
        DThreads::Yield();
    }
}

static void printMessageTimerLoop(std::string msg) {
    for (int i = 0; i < 5; i++) {
        printf("%s\n", msg.c_str());
        SleepEnsured(1, 0);
    }
}

/*
    This tests that yielding + scheduling + context switching works at a 
    minimal level where threads can alternate printing messages by calling yield
    themselves. First main will print, then the next thread, and the next, 
    looping back to main and repeating 5 times. N is the number of threads to 
    make, not including main.

    surrenderThroughTimer does the same test, but with the timer interrupt. 
    They must be cooperative, otherwise they can ignore the timer and keep 
    going.
*/
void surrenderThroughYield(int N) {
    for (int i = 1; i <= N; i++) {
        std::string name = "thread " + std::to_string(i);
        DThreads::Create(name, std::bind(printMessageYieldLoop, name));
    }
    printMessageYieldLoop("main");
}

void surrenderThroughTimer(int N) {
    for (int i = 1; i <= N; i++) {
        std::string name = "thread " + std::to_string(i);
        DThreads::Create(name, std::bind(printMessageTimerLoop, name));
    }
    printMessageTimerLoop("main");
}

/* -------------------------------------------------------------------------- */
static void mathyOperations() {
    const int N = 10000;
    int nums[N] = {0};
    for (int i = 0; i < N; i++) {
        nums[i] = i;
        // DThreads::CheckTimer();
    }

    /* Square all numbers. */
    for (int i = 0; i < N; i++) {
        nums[i] *= nums[i];
        // DThreads::CheckTimer();
    }

    /* Verify they are all correct. */
    for (int i = 0; i < N; i++) {
        assert(nums[i] == i * i);
        // DThreads::CheckTimer();
    }
}

/*
    Does some computations, yields the CPU, then continues computations. This
    tests that registers are being saved and restored correctly.
*/
void addIndependentVectors(int N) {
    srand(std::time(nullptr));

    std::list<tid_t> threads;
    for (int i = 0; i < N; i++) {
        threads.push_back(DThreads::Create("thread " + std::to_string(i), mathyOperations));
    }
    
    for (tid_t tid: threads) {
        DThreads::Join(tid);
    }

    printf("Passed!\n");
}

/* -------------------------------------------------------------------------- */

static void doSillyStuff() {
    for (int i = 0; i < 5; i++) {
        printf("pikmin\n");
        SleepEnsured(1, 0);
    }
}

/*
    Tests that Join() is correctly implemented such that waiting twice just 
    returns immediately and so does waiting on a TID of a non-existent thread.
    Should boil down to the same thing after joining once.
*/
void joinTwice() {
    tid_t tid = DThreads::Create("thread 1", doSillyStuff);

    DThreads::Join(tid);
    DThreads::Join(tid);

    /* If it gets here without crashing, success! */
    printf("Passed: joinTwice()!\n");
}

void joinFakeTID() {
    DThreads::Join(0xbeefcafe);
    printf("Passed: joinFakeTID()!\n");
}

/* -------------------------------------------------------------------------- */

Locks *lock;

const int sharedNum = 10000;
int shared[sharedNum];
void updateSharedCounter() {
    DThreads::CheckTimer();
    for (int i = 0; i < sharedNum; i++) {
        DThreads::CheckTimer();
        if (i % 2 == 0) {
            DThreads::CheckTimer();
            lock->Lock();
            shared[i] += 1;
            DThreads::CheckTimer();
            shared[i] += 1;
            lock->Unlock();
            DThreads::CheckTimer();
        } else {
            DThreads::CheckTimer();
            lock->Lock();
            shared[i] += 1;
            lock->Unlock();
            DThreads::CheckTimer();
        }
        DThreads::CheckTimer();
    }
}

/*
    Creates N threads that all update a shared counter while checking the timer
    interrupt. Odd indexes get incremented by 1 while even ones get incremented
    by two. There should be no lost updates.
*/
void testMutualExclusionAndJoin(int N) {
    std::vector<tid_t> threads; 
    for (int i = 0; i < N; i++) {
        threads.push_back(DThreads::Create("thread " + std::to_string(i), updateSharedCounter));
    }

    lock = DThreads::CreateLock();

    for (tid_t tid: threads) {
        DThreads::Join(tid);
    }

    for (int i = 0; i < sharedNum; i++) {
        if (i % 2 == 0) {
            assert(shared[i] == N*2);
        }
        else {
            assert(shared[i] == N);
        }
    }
    printf("testMutualExclusionAndJoin: Passed\n");
}

/* -------------------------------------------------------------------------- */

static void acquireLockAndPrint(Locks *lock, std::string name) {
    /* Let everyone attempt to acquire the lock before proceeding. */
    lock->Lock();
    DThreads::Yield();

    for (int i = 0; i < 3; i++) {
        printf("I am hoarding the lock! (%s)\n", name.c_str());
        SleepEnsured(1, 0);
    }

    printf("\n");
    lock->Unlock();
}

/*
    Creates a lock that each thread will try to acquire in their function. They
    will keep the lock for 3 seconds and print out their name. Correct output
    should be each thread name printed three times consecutively before 
    incrementing to the next thread. Should be FIFO order. To ensure blocking
    is tested properly, each thread will try to acquire the lock and succeed or 
    block, then yield the CPU to allow everyone else to try to grab it before
    succeeding.
*/
void testAcquiringLockBlocksOthers(int N) {
    Locks *lock = DThreads::CreateLock();

    /* Create threads and ensure only one ever prints. */
    std::vector<tid_t> threads; 
    for (int i = 1; i <= N; i++) {
        std::string name = "thread " + std::to_string(i);
        threads.push_back(DThreads::Create(name, std::bind(acquireLockAndPrint, lock, name)));
    }

    for (tid_t tid: threads) {
        DThreads::Join(tid);
    }
}

/* 
    Main will hold the lock so no one should be able to make any progress. 
    Tests that if someone is holding the lock, no one is able to progress in the
    meantime. Also tests that the idle thread correctly detects that all threads
    in the system are either blocked or terminated.
*/
void holdLockAndDontGiveUp(int N) {
    Locks *lock = DThreads::CreateLock();

    /* Create threads and ensure only one ever prints. */
    std::vector<tid_t> threads; 
    for (int i = 1; i <= N; i++) {
        std::string name = "thread " + std::to_string(i);
        threads.push_back(DThreads::Create(name, std::bind(acquireLockAndPrint, lock, name)));
    }

    lock->Lock();
    for (tid_t tid: threads) {
        DThreads::Join(tid);
    }
}
