#include <iostream>
#include <list>
#include <cassert>

#include "Manager.hpp"
#include "Thread.hpp"

std::list<Thread *> DThreadManager::threads;
bool DThreadManager::initialized = false;

void DThreadManager::Init() {
    // Initialize by adding main thread to the list of threads.
    std::cout << "Thread subsystem initialized.\n";

    // TODO
    initialized = true;
}

void DThreadManager::Create(std::string name, std::function<void()> func) {
    assert(initialized);

    Thread *t = new Thread(name, func);
    DThreadManager::threads.push_back(t);
    t->Start();

}

void DThreadManager::Yield() {
    assert(DThreadManager::initialized);

    // TODO: Make sure we are in a thread context.
    std::cout << "Thread yield called.\n";
}