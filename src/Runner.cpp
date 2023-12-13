#include <iostream>

#include "DThreads.hpp"
#include "Tests.hpp"
#include <cassert>
#include <cstdlib>
#include <ctime>
#include <pthread.h>

static void *mathyOperationsPthread(void *aux) {
    const int N = 10000;
    int nums[N] = {0};
    for (int i = 0; i < N; i++) {
        nums[i] = i;
    }

    /* Square all numbers. */
    for (int i = 0; i < N; i++) {
        nums[i] *= nums[i];
    }

    /* Verify they are all correct. */
    for (int i = 0; i < N; i++) {
        assert(nums[i] == i * i);
    }

    return nullptr;
}

/*
    Does some computations, yields the CPU, then continues computations. This
    tests that registers are being saved and restored correctly.
*/
void addIndependentVectorsPthread(int N) {
    srand(std::time(nullptr));

    std::list<pthread_t> threads;
    for (int i = 0; i < N; i++) {
        pthread_t tid;
        pthread_create(&tid, nullptr, mathyOperationsPthread, nullptr);
        threads.push_back(tid);
    }

    for (pthread_t tid: threads) {
        pthread_join(tid, nullptr);
    }
}

pthread_mutex_t lockPthread;
const int sharedNumPthread = 10000;
int sharedPthread[sharedNumPthread];
void *updateSharedCounter(void *aux) {
    for (int i = 0; i < sharedNumPthread; i++) {
        if (i % 2 == 0) {
            pthread_mutex_lock(&lockPthread);
            sharedPthread[i] += 1;
            sharedPthread[i] += 1;
            pthread_mutex_unlock(&lockPthread);
        } else {
            pthread_mutex_lock(&lockPthread);
            sharedPthread[i] += 1;
            pthread_mutex_unlock(&lockPthread);
        }
    }
    return nullptr;
}

/*
    Creates N threads that all update a shared counter while checking the timer
    interrupt. Odd indexes get incremented by 1 while even ones get incremented
    by two. There should be no lost updates.
*/
void testMutualExclusionAndJoinPthread(int N) {
    std::list<pthread_t> threads;
    for (int i = 0; i < N; i++) {
        pthread_t tid;
        pthread_create(&tid, nullptr, updateSharedCounter, nullptr);
        threads.push_back(tid);
    }

    pthread_mutex_init(&lockPthread, nullptr);

    for (pthread_t tid: threads) {
        pthread_join(tid, nullptr);
    }

    for (int i = 0; i < sharedNumPthread; i++) {
        if (i % 2 == 0) {
            assert(sharedPthread[i] == N*2);
        }
        else {
            assert(sharedPthread[i] == N);
        }
    }
    printf("testMutualExclusionAndJoinPthread: Passed\n");
}

int main(int argc, char *arv[]) {
    std::cout << "Runner program starting.\n\n";

    /* Run with pthreads. */
    if (argc == 2) {
        // addIndependentVectorsPthread(1000); // 0.035
        // testMutualExclusionAndJoinPthread(100); // 0.099
    } else {
        DThreads::Init();
        addIndependentVectors(1000); // 0.133
        // testMutualExclusionAndJoin(100); // 4.079
    }

    /* Correctness tests. */
    // testMainOverflow();
    // testThreadOverflow();
    // stackMappedCorrectly();
    // surrenderThroughYield(3);
    // surrenderThroughTimer(3);
    // addIndependentVectors();
    // joinTwice();
    // joinFakeTID();
    // testMutualExclusionAndJoin(10);
    // testAcquiringLockBlocksOthers(3);
    // holdLockAndDontGiveUp(3);

    std::cout << "\nRunner program ending.\n";
}