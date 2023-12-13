#include "Synch.hpp"
#include "DThreads.hpp"
#include <csignal>

Locks::Locks() {}
Locks::~Locks() {}


void Locks::Lock() {
    bool old = DThreads::SetInterrupts(false);

    Thread *t = DThreads::running;

    if (this->acquired) {
        this->waiters.push_back(t);
        t->Block();
    }

    this->acquired = true;

    DThreads::SetInterrupts(old);
}

void Locks::Unlock() {
    bool old = DThreads::SetInterrupts(false);

    if (waiters.size() > 0) {
        Thread *t = waiters.front();
        waiters.pop_front();

        t->Unblock();
    }

    this->acquired = false;

    DThreads::SetInterrupts(old);
}