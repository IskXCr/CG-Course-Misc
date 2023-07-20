//
// Created by LEI XU on 5/16/19.
//

#ifndef RAYTRACING_MATERIAL_H
#define RAYTRACING_MATERIAL_H

#include "Vector.hpp"

// Functions for geometric computations

// Compute reflection direction
static inline Vector3f reflect(const Vector3f &wo, const Vector3f &N)
{
    return wo - 2 * dotProduct(wo, N) * N;
}

// Compute refraction direction using Snell's law
// We need to handle with care the two possible situations:
//    - When the ray is inside the object
//    - When the ray is outside.
// If the ray is outside, you need to make cosi positive cosi = -N.I
// If the ray is inside, you need to invert the refractive indices and negate the normal N
static inline Vector3f refract(const Vector3f &wo, const Vector3f &N, const float &ior)
{
    float cosi = clamp(-1, 1, dotProduct(wo, N));
    float etai = 1, etat = ior;
    Vector3f n = N;
    if (cosi < 0)
    {
        cosi = -cosi;
    }
    else
    {
        std::swap(etai, etat);
        n = -N;
    }
    float eta = etai / etat;
    float k = 1 - eta * eta * (1 - cosi * cosi);
    return k < 0 ? 0 : eta * wo + (eta * cosi - sqrtf(k)) * n;
}

// Compute Fresnel equation
// \param I is the incident view direction
// \param N is the normal at the intersection point
// \param ior is the material refractive index
// \param[out] kr is the amount of light reflected
static inline void fresnel(const Vector3f &wo, const Vector3f &N, const float &ior, float &kr)
{
    float cosi = clamp(-1, 1, dotProduct(wo, N));
    float etai = 1, etat = ior;
    if (cosi > 0)
    {
        std::swap(etai, etat);
    }
    // Compute sini using Snell's law
    float sint = etai / etat * sqrtf(std::max(0.f, 1 - cosi * cosi));
    // Total internal reflection
    if (sint >= 1)
    {
        kr = 1;
    }
    else
    {
        float cost = sqrtf(std::max(0.f, 1 - sint * sint));
        cosi = fabsf(cosi);
        float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
        float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
        kr = (Rs * Rs + Rp * Rp) / 2;
    }
    // As a consequence of the conservation of energy, transmittance is given by:
    // kt = 1 - kr;
}

static inline Vector3f toWorld(const Vector3f &a, const Vector3f &N)
{
    Vector3f B, C;
    if (std::fabs(N.x) > std::fabs(N.y))
    {
        float invLen = 1.0f / std::sqrt(N.x * N.x + N.z * N.z);
        C = Vector3f(N.z * invLen, 0.0f, -N.x * invLen);
    }
    else
    {
        float invLen = 1.0f / std::sqrt(N.y * N.y + N.z * N.z);
        C = Vector3f(0.0f, N.z * invLen, -N.y * invLen);
    }
    B = crossProduct(C, N);
    return a.x * B + a.y * C + a.z * N;
}

static inline Vector3f toLocal(const Vector3f &a, const Vector3f &N)
{
    // TODO: Verify the correctness of this function
    Vector3f Np(-N.x, -N.y, N.z);
    return toWorld(a, Np);
}


// Materials

class MicrofacetDistribution
{
public:
    // Return the differential area, given the half vector wh.
    virtual float D(const Vector3f &wh) const = 0;
    // Return the value computed from the shadowing-masking function Lambda
    virtual float lambda(const Vector3f &w) const = 0;
    // Return the value of shadowing-masking term
    float G1(const Vector3f &w) const
    {
        return 1. / (1. + lambda(w));
    }
    float G(const Vector3f &wo, const Vector3f &wi) const
    {
        return 1.f / (1.f + lambda(wo) + lambda(wi));
    }
    // Sample an outgoing ray
    virtual Vector3f sample(const Vector3f &wo) const = 0;
    // Compute partial f without the fresnel term
    Vector3f partialFr(const Vector3f &wo, const Vector3f &wi) const;
    float getPdf(const Vector3f &wo, const Vector3f &wh) const;
    // Value close to zero correspond to near-perfect specular reflection
    static inline float roughtnessToAlpha(float roughness)
    {
        roughness = std::max(roughness, 1e-3f);
        float x = std::log(roughness);
        return 1.62142f + 0.819955f * x + 0.1734f * x * x + 0.0171201f * x * x * x +
               0.000640711f * x * x * x * x;
    }

protected:
    MicrofacetDistribution(bool sampleVisibleArea) : sampleVisibleArea(sampleVisibleArea) {}

    const bool sampleVisibleArea;

private:
};

class BeckmannDistribution : public MicrofacetDistribution
{
public:
    BeckmannDistribution(float alphax_, float alphay_, bool sampleVis = true) : MicrofacetDistribution(sampleVis), alphax(alphax_), alphay(alphay_) {}
    virtual float D(const Vector3f &wh) const override;
    virtual float lambda(const Vector3f &w) const override;
    virtual Vector3f sample(const Vector3f &wo) const override;

private:
    const float alphax, alphay;
};

class TrobridgeReitzDistribution : public MicrofacetDistribution
{
public:
    TrobridgeReitzDistribution(float alphax_, float alphay_, bool sampleVis = true) : MicrofacetDistribution(sampleVis), alphax(alphax_), alphay(alphay_) {}
    virtual float D(const Vector3f &wh) const override;
    virtual float lambda(const Vector3f &w) const override;
    virtual Vector3f sample(const Vector3f &wo) const override;

private:
    const float alphax, alphay;
};

enum MaterialType
{
    DIFFUSE,
    SPECULAR,
    TRANSLUCENT,
    MICROFACET,
};

class Material
{
public:
    MaterialType m_type; // Type of the material
    // Vector3f m_color;
    Vector3f m_emission;            // Emission
    float ior;                      // index of refraction
    Vector3f Kd;                    // K-Diffuse
    MicrofacetDistribution *mfDist; // MicrofacetDistribution
    // Fresnel fresnel;

    // Texture tex;
    friend class MicrofacetDistribution;

    inline Material(MaterialType t = DIFFUSE, Vector3f e = Vector3f(0, 0, 0), MicrofacetDistribution *m = nullptr);
    inline MaterialType getType();
    // inline Vector3f getColor();
    inline Vector3f getColorAt(double u, double v);
    inline Vector3f getEmission();
    inline bool hasEmission();

    // Sample a ray by material properties
    inline Vector3f sample(const Vector3f &wo, const Vector3f &N);
    // Given a ray, calculate the PDF of this ray
    inline float pdf(const Vector3f &wo, const Vector3f &wi, const Vector3f &N);
    // Given a ray, calculate the contribution of this ray in coefficients for component colors
    inline Vector3f eval(const Vector3f &wo, const Vector3f &wi, const Vector3f &N);
};

Material::Material(MaterialType t, Vector3f e, MicrofacetDistribution *m)
{
    m_type = t;
    // m_color = c;
    m_emission = e;
    mfDist = m;
}

MaterialType Material::getType() { return m_type; }

/// Vector3f Material::getColor(){return m_color;}
Vector3f Material::getEmission() { return m_emission; }

bool Material::hasEmission()
{
    if (m_emission.norm() > EPSILON)
        return true;
    else
        return false;
}

Vector3f Material::getColorAt(double u, double v)
{
    return Vector3f();
}

Vector3f Material::sample(const Vector3f &wo, const Vector3f &N)
{
    switch (m_type)
    {
    case DIFFUSE:
    {
        // uniform sample on the hemisphere
        float x_1 = get_random_float(), x_2 = get_random_float();
        float z = std::fabs(1.0f - 2.0f * x_1);
        float r = std::sqrt(1.0f - z * z), phi = 2 * M_PI * x_2;
        Vector3f localRay(r * std::cos(phi), r * std::sin(phi), (dotProduct(wo, N) > 0) ? z : -z);
        return toWorld(localRay, N);
        break;
    }
    case SPECULAR:
    {
    }
    case MICROFACET:
    {
        if (mfDist == nullptr)
            throw std::runtime_error("Invalid Microfacet BRDF: No specified distribution.");
        return toWorld(mfDist->sample(toLocal(wo, N)), N);
        break;
    }
    }
}

float Material::pdf(const Vector3f &wo, const Vector3f &wi, const Vector3f &N)
{
    switch (m_type)
    {
    case DIFFUSE:
    {
        // uniform sample probability 1 / (2 * PI)
        if (dotProduct(wi, N) * dotProduct(wo, N) > 0.0f)
            return 0.5f / M_PI;
        else
            return 0.0f;
        break;
    }
    case SPECULAR:
    {
    }
    case MICROFACET:
    {
        if (mfDist == nullptr)
            throw std::runtime_error("Invalid Microfacet BRDF: No specified distribution.");
        if (dotProduct(wi, N) > 0.0f)
            return mfDist->getPdf(toLocal(wo, N), toLocal((wi + wo).normalized(), N));
        else
            return 0.0f;
        break;
    }
    }
}

// wi, wo and N must point outwards (relative to the point being evaluated)
// For direct illumination, wo is the ray from camera.
Vector3f Material::eval(const Vector3f &wo, const Vector3f &wi, const Vector3f &N)
{
    switch (m_type)
    {
    case DIFFUSE:
    {
        // calculate the contribution of diffuse model
        if (dotProduct(wi, N) * dotProduct(wo, N) > 0.0f)
        {
            Vector3f diffuse = Kd / M_PI;
            return diffuse;
        }
        else
            return Vector3f(0.0f);
        break;
    }
    case SPECULAR:
    {
    }
    case MICROFACET:
    {
        if (mfDist == nullptr)
            throw std::runtime_error("Invalid Microfacet BRDF: No specified distribution.");
        float cosalpha = dotProduct(N, wi);
        if (cosalpha > 0.0f)
        {
            float kr;
            fresnel(wo, N, this->ior, kr);
            return kr * mfDist->partialFr(toLocal(wo, N), toLocal(wi, N));
        }
        else
            return Vector3f(0.0f);
        break;
    }
    }
}

#endif // RAYTRACING_MATERIAL_H
