//
// Created by LEI XU on 5/13/19.
//
#pragma once
#ifndef RAYTRACING_OBJECT_H
#define RAYTRACING_OBJECT_H

#include "Vector.hpp"
#include "global.hpp"
#include "Bounds3.hpp"
#include "Ray.hpp"
#include "Intersection.hpp"

class Object
{
public:
    Object() {}
    virtual ~Object() {}
    virtual bool intersect(const Ray &ray) = 0;
    virtual bool intersect(const Ray &ray, float &, uint32_t &) const = 0;
    virtual Intersection getIntersection(Ray _ray) = 0;
    virtual void getSurfaceProperties(const Vector3f &, const Vector3f &, const uint32_t &, const Vector2f &, Vector3f &, Vector2f &) const = 0;
    virtual Bounds3 getBounds() = 0;
    virtual float getArea() = 0;

    // Sample the bounding box at the point.
    // The object should be responsible for changing the pdf.
    // If the called object is a complex or hiearchy, it should return the correct pdf.
    // \return modified Intersection object containing position information and normal information,
    //         and modified pdf for division. 
    virtual void sample(Intersection &pos, float &pdf) = 0;
    virtual bool hasEmit() = 0;
};

#endif // RAYTRACING_OBJECT_H
