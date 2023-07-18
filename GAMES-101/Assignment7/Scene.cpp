//
// Created by Göksu Güvendiren on 2019-05-14.
//

#include "Scene.hpp"
// #include "Triangle.hpp"

void Scene::buildBVH()
{
    this->bvh = new BVHAccel(objects);
}

Intersection Scene::intersect(const Ray &ray) const
{
    return this->bvh->Intersect(ray);
}

// Sample the light sources from a given point.
// 1. Accumulate the total area of all light sources.
// 2. With distribution based on area, randomly select a light source to sample.
void Scene::sampleLight(Intersection &pos, float &pdf) const
{
    float emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k)
    {
        if (objects[k]->hasEmit())
        {
            emit_area_sum += objects[k]->getArea();
        }
    }
    float p = get_random_float() * emit_area_sum;
    emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k)
    {
        if (objects[k]->hasEmit())
        {
            emit_area_sum += objects[k]->getArea();
            if (p <= emit_area_sum)
            {
                objects[k]->Sample(pos, pdf);
                break;
            }
        }
    }
}

// Unused, and shouldn't be used (no BVH)
bool Scene::trace(const Ray &ray,
                  const std::vector<Object *> &objects,
                  float &tNear) const
{
    bool t = false;
    for (uint32_t k = 0; k < objects.size(); ++k)
    {
        if (Intersection isect = objects[k]->getIntersection(ray); isect.happened && isect.distance < tNear)
        {
            tNear = isect.distance;
            t |= true;
        }
    }

    return t;
}

// Implementation of Path Tracing
Vector3f Scene::castRay(const Ray &ray, int depth) const
{
    // TODO: Implement Path Tracing Algorithm here
    // Do not use maxDepth as this cuts off energy!
    // CAUTION: Be careful of potential problem in fp precision!
    // if (depth > 0)
        // return Vector3f(0.f);

    // 1. Find where this ray has hit
    Vector3f hitColor;

    Intersection isect = intersect(ray); // The intersection between argument ray and this Scene
    if (isect.happened)
    {
        // hitColor = Vector3f(166.f / isect.distance);
        // return hitColor;

        Vector3f wIn = Vector3f() - ray.direction;
        Vector3f &hitPos = isect.coords;
        Vector3f &hitNormal = isect.normal;
        Object *hitObj = isect.obj;
        Material *hitMaterial = isect.m;

        Intersection lightPos;
        float pdfLight;
        sampleLight(lightPos, pdfLight);
        Vector3f &lightCoords = lightPos.coords;
        Ray lightOut = Ray(hitPos, normalize(lightCoords - hitPos));
        Vector3f &wLightOut = lightOut.direction;

        if (dotProduct(wLightOut, hitNormal) > EPSILON) // Must face the light
        {
            if (Intersection isect0 = intersect(lightOut);
                isect0.happened && isect0.obj == lightPos.obj) // and has no blocking in between, or there is another light source
            {
                Vector3f &lightNormal = isect0.normal;
                Vector3f &lightEmit = isect0.emit;

                float frac = isect0.distance * isect0.distance * pdfLight;
                Vector3f fr = hitMaterial->eval(wIn, wLightOut, hitNormal);
                float ctheta = dotProduct(wLightOut, hitNormal);
                float cthetap = dotProduct(Vector3f() - wLightOut, lightNormal);
                hitColor += lightEmit * fr * (ctheta * cthetap / frac);
                // hitColor += Vector3f{1.0} / frac;
                // hitColor += Vector3f{0.5};
                // fprintf(stderr, "frac0=%f, frac=%f\n", frac0, frac);
            }
        }

        if (get_random_float() <= RussianRoulette)
        {
            Vector3f wOut = hitMaterial->sample(wIn, hitNormal);
            Ray outRay(hitPos, wOut.normalized());
            float pdfMaterial = hitMaterial->pdf(wIn, wOut, hitNormal);

            if (Intersection isect0 = intersect(outRay);
                isect0.happened && !isect0.m->hasEmission())
            {
                float frac = pdfMaterial * RussianRoulette;
                Vector3f shade = castRay(outRay, depth + 1).regularized();
                Vector3f f_r = hitMaterial->eval(wIn, wOut, hitNormal);
                float ctheta = dotProduct(wOut, hitNormal);
                hitColor +=  shade * f_r * ctheta / frac;
            }
        }

        if (isect.m->hasEmission())
        {
            hitColor += isect.m->getEmission();
        }
    }

    return hitColor;
}