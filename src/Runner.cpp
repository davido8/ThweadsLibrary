#include <iostream>

#include "DThreads.hpp"
#include "Tests.hpp"

void thread_func() {
    std::cout << "Hello from thread!\n";
    DThreads::Yield();
}

int main(int argc, char *arv[]) {
    std::cout << "Runner program starting.\n\n";

    DThreads::Init();
    DThreads::Create("thread 1", thread_func);

    std::cout << "\nRunner program ending.\n";
}