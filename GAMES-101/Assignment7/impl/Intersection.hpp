//
// Created by LEI XU on 5/16/19.
//

#ifndef RAYTRACING_INTERSECTION_H
#define RAYTRACING_INTERSECTION_H
#include "Vector.hpp"
#include "Material.hpp"
class Object;
class Sphere;

struct Intersection
{
    Intersection()
    {
        happened = false;
        coords = Vector3f();
        normal = Vector3f();
        distance = std::numeric_limits<double>::max();
        obj = nullptr;
        m = nullptr;
    }
    bool happened;    // Whether the intersection has happened
    Vector3f coords;  // The coordinate where the intersection occurred
    Vector3f tcoords; // Unknown
    Vector3f normal;  // The surface normal at the point of intersection
    Vector3f emit;    // The emission of the target. Only valid after intersection.
    double distance;  // Records t, where the actual coord = ori + dir * t. Default to std::numeric_limits<double>::max()
    Object *obj;      // The target object with which the ray intersected
    Material *m;      // The material of target object
};
#endif // RAYTRACING_INTERSECTION_H
