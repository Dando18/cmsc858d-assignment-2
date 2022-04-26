/**
 * @file utilities.h
 * @author Daniel Nichols
 * @brief 
 * @date 2022-04-13
 */
#pragma once

/* stl includes */
#include <chrono>
#include <execution>

/* tpl includes */
#include "libsais.h"

namespace utilities {

#if defined(_OPENMP)
constexpr auto executionPolicy = std::execution::par_unseq;
#else
constexpr auto executionPolicy = std::execution::seq;
#endif


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

template <typename Iterator>
int32_t LCPLength(Iterator sBegin, Iterator sEnd, Iterator tBegin) {
    auto const& [s, t] = std::mismatch(sBegin, sEnd, tBegin);
    return std::min(std::distance(sBegin, s), std::distance(tBegin, t));
}


}   // namespace utilities