#include <iostream>

#include "DThreads.hpp"

void thread_func() {
    std::cout << "Hello from thread!\n";
    DThreads::Yield();
}

int main(int argc, char *arv[]) {
    std::cout << "Runner program starting.\n";

    DThreads::Init();
    // DThreadManager::Create("Thread 1", thread_func);
    // DThreadManager::Create("Thread 2", thread_func);
    // DThreadManager::Create("Thread 3", thread_func);
    // DThreadManager::Create("Thread 4", thread_func);
    // DThreadManager::Create("Thread 5", thread_func);

    std::cout << "Runner program ending.\n";
}