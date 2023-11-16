#ifndef THREAD_HPP
#define THREAD_HPP

#include <string>
#include <functional>

class Thread {
    private:
        std::string name;
        std::function<void()> main;

    public:
        Thread(
            std::string name,
            std::function<void()> func
        );
        ~Thread();

        void Start();
};

#endif