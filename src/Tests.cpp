#include <cstdio>
#include <stdint.h>
#include <string>
#include "DThreads.hpp"

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

static void printMessageYieldLoopM() {
    while (true) {
        printf("main\n");
        DThreads::Yield();
    }
}

static void printMessageYieldLoop1() {
    while (true) {
        printf("thread 1\n");
        DThreads::Yield();
    }
}

static void printMessageYieldLoop2() {
    while (true) {
        printf("thread 2\n");
        DThreads::Yield();
    }
}

/*
    TODO: Change to be able to take parameters and differing N.
*/
void willinglyYielding(int N) {
    DThreads::Create("thread 1", printMessageYieldLoop1);
    DThreads::Create("thread 2", printMessageYieldLoop2);
    printMessageYieldLoopM();
}
