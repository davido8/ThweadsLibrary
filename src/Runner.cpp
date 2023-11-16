#include <iostream>

#include "Manager.hpp"

void thread_func() {
    std::cout << "Hello from thread!\n";
    DThreadManager::Yield();
}

int main(int argc, char *arv[]) {
    std::cout << "Runner program starting.\n";

    DThreadManager::Init();
    DThreadManager::Create("Thread 1", thread_func);

    std::cout << "Runner program ending.\n";
}