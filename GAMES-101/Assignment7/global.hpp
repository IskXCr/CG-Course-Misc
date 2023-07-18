#pragma once
#include <iostream>
#include <cmath>
#include <random>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <future>
#include <functional>
#include <chrono>

#undef M_PI
#define M_PI 3.141592653589793f

extern const float EPSILON;
const float kInfinity = std::numeric_limits<float>::max();

inline float clamp(const float &lo, const float &hi, const float &v)
{
    return std::max(lo, std::min(hi, v));
}

inline bool solveQuadratic(const float &a, const float &b, const float &c, float &x0, float &x1)
{
    float discr = b * b - 4 * a * c;
    if (discr < 0)
        return false;
    else if (std::fabs(discr) < EPSILON)
        x0 = x1 = -0.5 * b / a;
    else
    {
        float q = (b > 0) ? -0.5 * (b + std::sqrt(discr)) : -0.5 * (b - std::sqrt(discr));
        x0 = q / a;
        x1 = c / q;
    }
    if (x0 > x1)
        std::swap(x0, x1);
    return true;
}

extern thread_local std::random_device dev;
extern thread_local std::mt19937 rng;
extern thread_local std::uniform_real_distribution<float> dist; // distribution in range [0, 1]

inline float get_random_float()
{
    return dist(rng);
}
