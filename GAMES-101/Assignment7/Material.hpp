//
// Created by LEI XU on 5/16/19.
//

#ifndef RAYTRACING_MATERIAL_H
#define RAYTRACING_MATERIAL_H

#include "Vector.hpp"

// Utility functions

// Compute the square root of each component
static inline Vector3f sqrt(const Vector3f &v)
{
    float x, y, z;
    x = std::sqrt(v.x);
    y = std::sqrt(v.y);
    z = std::sqrt(v.z);
    if (std::isnan(x) || std::isnan(y) || std::isnan(z))
        throw std::runtime_error("Invalid sqrt on vector v");
    return Vector3f(x, y, z);
}

// Convert to world coordinate
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

// Convert to local coordinate around N
static inline Vector3f toLocal(const Vector3f &a, const Vector3f &N)
{
    // TODO: Verify the correctness of this function
    Vector3f Np(-N.x, -N.y, N.z);
    return toWorld(a, Np);
}

// Compute reflection direction. The result points outwards on the hemisphere.
static inline Vector3f reflect(const Vector3f &wo, const Vector3f &N)
{
    return 2 * dotProduct(wo, N) * N - wo;
}

// Compute refraction direction using Snell's law. The result points outwards on the sphere.
// \param eta the relative (to the air) index of refraction of the object
// \param N the normal of the surface. By default, the normal points toward outside of the object.
static inline Vector3f refract(const Vector3f &wo, const Vector3f &N, float eta)
{
    float cosi = clamp(-1.0f, 1.0f, dotProduct(wo, N));
    Vector3f n = N;
    if (cosi < 0.0f)
    {
        cosi = -cosi;
    }
    else
    {
        eta = 1. / eta;
        n = -N;
    }
    float k = 1 - eta * eta * (1 - cosi * cosi); // test total internal reflection
    return k < 0.0f ? Vector3f(0.0f) : -eta * wo + (std::sqrt(k) - eta * cosi) * n;
}

// Compute Fresnel equation for dielectrics. Return the portion of spectrum reflected.
// \param cosThetaI the cosine of the incident angle. May be negative if N and wi is not on the same side
// \param eta is the relative refractive index of the incident material
static inline float frDielectric(const float cosThetaI, const float &eta)
{
    float kr;
    float cosi = clamp(-1, 1, cosThetaI);
    float etai = 1.0f, etat = eta;
    if (cosi > 0.0f)
    {
        std::swap(etai, etat);
    }
    // Compute sini using Snell's law
    float sint = etai / etat * std::sqrt(std::max(0.f, 1.0f - cosi * cosi));
    // Total internal reflection
    if (sint >= 1.0f)
    {
        kr = 1.0f;
    }
    else
    {
        float cost = std::sqrt(std::max(0.f, 1.0f - sint * sint));
        cosi = std::abs(cosi);
        float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
        float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
        kr = (Rs * Rs + Rp * Rp) / 2.0f;
    }
    // As a consequence of the conservation of energy, transmittance is given by:
    // kt = 1 - kr;
    return kr;
}

// Compute Fresnel equation for conductors. Return the portion of spectrum reflected.
// \param cosThetaI the cosine of the incident angle. May be negative if N and wi is not on the same side
// \param eta is the relative refractive index of the material, for each rgb channel
// \param k is the absorption coefficient, for each rgb channel
static inline Vector3f frConductor(float cosThetaI, Vector3f eta, Vector3f k)
{
    cosThetaI = std::abs(cosThetaI);
    cosThetaI = clamp(-1, 1, cosThetaI);
    Vector3f etak = k;

    float cosThetaI2 = cosThetaI * cosThetaI;
    float sinThetaI2 = 1. - cosThetaI2;
    Vector3f eta2 = eta * eta;
    Vector3f etak2 = etak * etak;

    Vector3f t0 = eta2 - etak2 - sinThetaI2;
    Vector3f a2plusb2 = sqrt(t0 * t0 + 4 * eta2 * etak2);
    Vector3f t1 = a2plusb2 + cosThetaI2;
    Vector3f a = sqrt(0.5f * (a2plusb2 + t0));
    Vector3f t2 = (float)2 * cosThetaI * a;
    Vector3f Rs = (t1 - t2) / (t1 + t2);

    Vector3f t3 = cosThetaI2 * a2plusb2 + sinThetaI2 * sinThetaI2;
    Vector3f t4 = t2 * sinThetaI2;
    Vector3f Rp = Rs * (t3 - t4) / (t3 + t4);

    return 0.5 * (Rp + Rs);
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

class Fresnel
{
public:
    // Given the cosine of the angle made by the incoming direction and the surface normal,
    // return the amount of light reflected by the surface.
    virtual Vector3f eval(float cosThetaI) const = 0;
};

class FresnelConductor : public Fresnel
{
public:
    virtual Vector3f eval(float cosThetaI) const override;
    FresnelConductor(const Vector3f &eta, const Vector3f &k) : eta(eta), k(k) {}

private:
    Vector3f eta, k;
};

class FresnelDielectric : public Fresnel
{
public:
    virtual Vector3f eval(float cosThetaI) const override;
    FresnelDielectric(const Vector3f &eta) : eta(eta) {}

private:
    Vector3f eta;
};

class FresnelNoOp : public Fresnel
{
public:
    virtual Vector3f eval(float cosThetaI) const override { return Vector3f(1.0f); }

private:
};

enum MaterialType
{
    DIFFUSE = (1 << 0),              // Lambertian diffuse
    SPECULAR_TEST = (1 << 1),        // Perfect specular (FP precision problem still exist)
    DIELETRIC_TEST = (1 << 2),       // Dielectric simulation
    FRESNEL_REFLECTION = (1 << 4),   // Specular fresnel materials
    FRESNEL_TRANSMISSION = (1 << 5), // Fresnel transmission
    FRESNEL_SPECULAR = (1 << 6),     // Fresnel specular
    MICROFACET = (1 << 8)            // Microfacet simulation
};

class Material
{
public:
    MaterialType m_type; // Type of the material
    // Vector3f m_color;
    Vector3f m_emission;            // Emission for light sources
    Vector3f Kd;                    // K-Diffuse
    Vector3f eta;                   // FRESNEL & Other materials - Index of refraction
    Vector3f Krefl;                 // REFLECTION - Scale the spectrum for fresnel reflection
    Vector3f Ktrans;                // TRANSMISSION - Scale the spectrum for fresnel transmission
    Fresnel *fresnel;               // FRESNEL - For fresnel materials
    MicrofacetDistribution *mfDist; // MicrofacetDistribution

    // Texture tex;
    friend class MicrofacetDistribution;

    inline Material(MaterialType t = DIFFUSE, Vector3f e = Vector3f(0, 0, 0));
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

Material::Material(MaterialType t, Vector3f e)
{
    m_type = t;
    // m_color = c;
    Kd = Vector3f(1.0f, 1.0f, 1.0f);
    Krefl = Vector3f(1.0f, 1.0f, 1.0f);
    Ktrans = Vector3f(1.0f, 1.0f, 1.0f);
    m_emission = e;
    fresnel = new FresnelNoOp();
    mfDist = nullptr;
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
    if (wo.isZero())
        return {0, 0, 0};
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
    case SPECULAR_TEST:
    {
        return reflect(wo, N);
        break;
    }
    case DIELETRIC_TEST:
    {
        if (get_random_float() <= frDielectric(dotProduct(wo, N), eta[0])) // reflect
        {
            return reflect(wo, N);
        }
        else // refract
        {
            return refract(wo, N, eta[0]);
        }
        break;
    }
    case FRESNEL_REFLECTION:
    {
        return reflect(wo, N);
        break;
    }
    case FRESNEL_TRANSMISSION:
    {
        return refract(wo, N, eta[0]);
        break;
    }
    case FRESNEL_SPECULAR:
    {
        Vector3f reflected = reflect(wo, N);
        Vector3f refracted = refract(wo, N, eta[0]);
        float F = frDielectric(dotProduct(wo, N), eta[0]);
        if (get_random_float() < F)
            return reflected;
        else 
            return refracted;
        break;
    }
    case MICROFACET:
    {
        if (mfDist == nullptr)
            throw std::runtime_error("Invalid Microfacet BRDF: No specified distribution.");
        break;
    }
    }
    return 0.0f;
}

float Material::pdf(const Vector3f &wo, const Vector3f &wi, const Vector3f &N)
{
    if (wo.isZero())
        return 0;
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
    case SPECULAR_TEST:
    {
        if (dotProduct(wo, reflect(wi, N)) > 1.0f - EPSILON)
            return 1.0f;
        else
            return 0.0f;
    }
    case DIELETRIC_TEST:
    {
        float prob = frDielectric(dotProduct(wi, N), eta[0]);
        Vector3f reflected = reflect(wo, N);
        Vector3f refracted = refract(wo, N, eta[0]);
        if (dotProduct(reflected, wi) > 1.0f - EPSILON)
        {
            return prob;
        }
        else if (dotProduct(refracted, wi) > 1.0f - EPSILON)
        {
            return std::max(0.0f, 1.0f - prob);
        }
        else
        {
            return 0.0f;
        }

        break;
    }
    case FRESNEL_REFLECTION:
    {
        Vector3f reflected = reflect(wo, N);
        if (dotProduct(reflected, wi) > 1.0f - EPSILON)
            return 1;
        else
            return 0;
        break;
    }
    case FRESNEL_TRANSMISSION:
    {
        Vector3f refracted = refract(wo, N, eta[0]);
        if (dotProduct(refracted, wi) > 1.0f - EPSILON)
            return 1;
        else
            return 0;
        break;
    }
    case FRESNEL_SPECULAR:
    {
        Vector3f reflected = reflect(wo, N);
        Vector3f refracted = refract(wo, N, eta[0]);
        float F = frDielectric(dotProduct(wo, N), eta[0]);
        if (dotProduct(reflected, wi) > 1.0f - EPSILON)
            return F;
        else if (dotProduct(refracted, wi) > 1.0f - EPSILON)
            return 1 - F;
        else
            return 0;
        break;
    }
    case MICROFACET:
    {
        if (mfDist == nullptr)
            throw std::runtime_error("Invalid Microfacet BRDF: No specified distribution.");
        break;
    }
    }
    return 0.0f;
}

// wi, wo and N must point outwards (relative to the point being evaluated)
// For direct illumination, wo is the ray from camera.
Vector3f Material::eval(const Vector3f &wo, const Vector3f &wi, const Vector3f &N)
{
    if (wo.isZero())
        return {0, 0, 0};
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
            return Vector3f::zero();
        break;
    }
    case SPECULAR_TEST:
    {
        if (dotProduct(wo, reflect(wi, N)) > 1.0f - EPSILON)
            return Krefl;
        else
            return Vector3f::zero();
    }
    case DIELETRIC_TEST:
    {
        float prob = frDielectric(dotProduct(wi, N), eta[0]);
        Vector3f reflected = reflect(wo, N);
        Vector3f refracted = refract(wo, N, eta[0]);
        if (dotProduct(reflected, wi) > 1.0f - EPSILON)
        {
            return Vector3f(prob);
        }
        else if (dotProduct(refracted, wi) > 1.0f - EPSILON)
        {
            return Vector3f(std::max(0.0f, 1.0f - prob));
        }
        else
        {
            return Vector3f::zero();
        }
    }
    case FRESNEL_REFLECTION:
    {
        Vector3f reflected = reflect(wo, N);
        if (dotProduct(reflected, wi) > 1.0f - EPSILON)
            return fresnel->eval(dotProduct(wo, N)) / std::abs(dotProduct(wo, N));
        else
            return Vector3f::zero();
        break;
    }
    case FRESNEL_TRANSMISSION:
    {
        Vector3f refracted = refract(wo, N, eta[0]);
        if (dotProduct(refracted, wi) > 1.0f - EPSILON)
        {
            Vector3f ft = Ktrans * Vector3f(1.0f) - fresnel->eval(dotProduct(wo, N));
            return ft / std::abs(dotProduct(wo, N));
        }
        else
            return Vector3f::zero();
        break;
    }
    case FRESNEL_SPECULAR:
    {
        Vector3f reflected = reflect(wo, N);
        Vector3f refracted = refract(wo, N, eta[0]);
        float F = frDielectric(dotProduct(wo, N), eta[0]);
        if (dotProduct(reflected, wi) > 1.0f - EPSILON)
        {
            return fresnel->eval(dotProduct(wo, N)) / std::abs(dotProduct(wo, N));
        }
        else if (dotProduct(refracted, wi) > 1.0f - EPSILON)
        {
            Vector3f ft = Ktrans * Vector3f(1.0f) - fresnel->eval(dotProduct(wo, N));
            return ft / std::abs(dotProduct(wo, N));
        }
        else
        {
            return Vector3f::zero();
        }
        break;
    }
    case MICROFACET:
    {
        if (mfDist == nullptr)
            throw std::runtime_error("Invalid Microfacet BRDF: No specified distribution.");
        break;
    }
    }
    return Vector3f::zero();
}

#endif // RAYTRACING_MATERIAL_H
