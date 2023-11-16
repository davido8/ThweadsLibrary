#include <string>
#include <iostream>
#include <functional>

#include "Thread.hpp"


Thread::Thread(std::string name, std::function<void()> func) {
    std::cout << "Creating thread " << name << "\n";

    this->name = name;
    this->main = func;
}

void Thread::Start() {
    main();
}