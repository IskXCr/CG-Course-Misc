//
// Created by LEI XU on 5/13/19.
//
#pragma once
#ifndef RAYTRACING_VECTOR_H
#define RAYTRACING_VECTOR_H

#include "global.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>

class Vector3f
{
public:
    float x, y, z;
    Vector3f() : x(0.f), y(0.f), z(0.f) {}
    Vector3f(float xx) : x(xx), y(xx), z(xx) {}
    Vector3f(float xx, float yy, float zz) : x(xx), y(yy), z(zz) {}
    Vector3f operator*(const float &r) const { return Vector3f(x * r, y * r, z * r); }
    Vector3f operator/(const float &r) const { return Vector3f(x / r, y / r, z / r); }

    float norm() const { return std::sqrt(x * x + y * y + z * z); }
    bool isZero() const
    {
        return x == 0.f && y == 0.f && z == 0.f;
    }
    inline Vector3f normalized() const
    {
        float n = std::sqrt(x * x + y * y + z * z);
        return Vector3f(x / n, y / n, z / n);
    }
    // Regularize the vector such that all components reside in [0, 1]
    inline Vector3f regularized() const
    {
        return Vector3f(clamp(0.f, 1., x), clamp(0.f, 1., y), clamp(0.f, 1., z));
    }
    inline Vector3f reversed() const
    {
        return Vector3f(-x, -y, -z);
    }

    Vector3f operator*(const Vector3f &v) const { return Vector3f(x * v.x, y * v.y, z * v.z); }
    Vector3f operator-(const Vector3f &v) const { return Vector3f(x - v.x, y - v.y, z - v.z); }
    Vector3f operator+(const Vector3f &v) const { return Vector3f(x + v.x, y + v.y, z + v.z); }
    Vector3f operator/(const Vector3f &v) const { return Vector3f(x / v.x, y / v.y, z / v.z); }
    Vector3f operator-() const { return Vector3f(-x, -y, -z); }
    Vector3f &operator+=(const Vector3f &v)
    {
        x += v.x, y += v.y, z += v.z;
        return *this;
    }

    friend Vector3f operator*(const float &r, const Vector3f &v)
    {
        return Vector3f(v.x * r, v.y * r, v.z * r);
    }

    friend std::ostream &operator<<(std::ostream &os, const Vector3f &v)
    {
        return os << v.x << ", " << v.y << ", " << v.z;
    }

    float operator[](int index) const;
    float &operator[](int index);

    static inline Vector3f minVector(const Vector3f &p1, const Vector3f &p2)
    {
        return Vector3f(std::min(p1.x, p2.x), std::min(p1.y, p2.y),
                        std::min(p1.z, p2.z));
    }

    static inline Vector3f maxVector(const Vector3f &p1, const Vector3f &p2)
    {
        return Vector3f(std::max(p1.x, p2.x), std::max(p1.y, p2.y),
                        std::max(p1.z, p2.z));
    }

    static inline Vector3f zero()
    {
        return Vector3f(0.0f);
    }

    static inline bool isZero(Vector3f v)
    {
        return v.isZero();
    }
};

inline float Vector3f::operator[](int index) const
{
    return (&x)[index];
}

inline float &Vector3f::operator[](int index)
{
    return (&x)[index];
}

class Vector2f
{
public:
    Vector2f() : x(0), y(0) {}
    Vector2f(float xx) : x(xx), y(xx) {}
    Vector2f(float xx, float yy) : x(xx), y(yy) {}
    Vector2f operator*(const float &r) const { return Vector2f(x * r, y * r); }
    Vector2f operator+(const Vector2f &v) const { return Vector2f(x + v.x, y + v.y); }
    float x, y;
};

inline Vector3f lerp(const Vector3f &a, const Vector3f &b, const float &t)
{
    return a * (1 - t) + b * t;
}

inline Vector3f normalize(const Vector3f &v)
{
    float mag2 = v.x * v.x + v.y * v.y + v.z * v.z;
    if (mag2 > 0)
    {
        float invMag = 1 / sqrtf(mag2);
        return Vector3f(v.x * invMag, v.y * invMag, v.z * invMag);
    }

    return v;
}

inline float dotProduct(const Vector3f &a, const Vector3f &b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline Vector3f crossProduct(const Vector3f &a, const Vector3f &b)
{
    return Vector3f(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x);
}

#endif // RAYTRACING_VECTOR_H
