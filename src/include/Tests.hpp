
#ifndef TESTS_HPP
#define TESTS_HPP

/*
    Each thread reserves an amount of memory for its stack. At the end of this 
    memory is a page with no permissions so that if they overflow they crash
    instead of running into someone else's stack. This tests to ensure that it
    properly crashes.

    Note: To ensure that it is segfaulting where it should be, modify source
    code to print out the last valid address.
*/
void testMainOverflow();
void testThreadOverflow();

/*
    Tests that setting %rsp to the newly mmap'ed stack doesn't result in a
    segmentation fault. And when you don't mmap it, it does. 

    Note: Requires modifying source code to not mmap page.
*/
void stackMappedCorrectly();

/*
    This tests that yielding + scheduling + context switching works at a 
    minimal level where threads can alternate printing messages by calling yield
    themselves. First main will print, then the next thread, and the next, 
    looping back to main and repeating forever. N is the number of threads to 
    make, not including main.

    surrenderThroughTimer does the same test, but with the timer interrupt. 
    They must be cooperative, otherwise they can ignore the timer and keep 
    going.
*/
void surrenderThroughYield(int N);
void surrenderThroughTimer(int N);

/*
    Does some computations, yields the CPU, then continues computations. This
    tests that registers are being saved and restored correctly.
*/
void addIndependentVectors(int N);

/*
    Tests that Join() is correctly implemented such that waiting twice just 
    returns immediately and so does waiting on a TID of a non-existent thread.
    Should boil down to the same thing after joining once.
*/
void joinTwice();
void joinFakeTID();

/*
    Creates N threads that all update a shared counter while checking the timer
    interrupt. Odd indexes get incremented by 1 while even ones get incremented
    by two. There should be no lost updates.
*/
void testMutualExclusionAndJoin(int N);


void holdLockAndDontGiveUp(int N);
void testAcquiringLockBlocksOthers(int N);


#endif