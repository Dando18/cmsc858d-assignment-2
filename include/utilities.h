/**
 * @file utilities.h
 * @author Daniel Nichols
 * @brief 
 * @date 2022-04-13
 */
#pragma once

/* stl includes */
#include <chrono>

/* tpl includes */
#include "libsais.h"

namespace utilities {

class Timer {
public:
    void start() {
        begin_ = std::chrono::high_resolution_clock::now();
    }

    void stop() {
        end_ = std::chrono::high_resolution_clock::now();
    }

    template <typename DurationType>
    double elapsed() {
        return std::chrono::duration_cast<DurationType>(end_ - begin_).count();
    }

    double secondsElapsed() {
        return elapsed<std::chrono::seconds>();
    }

    double millisecondsElapsed() {
        return elapsed<std::chrono::milliseconds>();
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> begin_, end_;
};


}   // namespace utilities