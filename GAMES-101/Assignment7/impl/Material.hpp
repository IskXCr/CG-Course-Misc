//
// Created by LEI XU on 5/16/19.
//

#ifndef RAYTRACING_MATERIAL_H
#define RAYTRACING_MATERIAL_H

#include "Vector.hpp"

// ======================== Beginning of Utility Functions

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

static inline std::tuple<Vector3f, Vector3f> getLocalBasis(const Vector3f &N)
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
    return {B, C};
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
    auto [B, C] = getLocalBasis(N);

    return {dotProduct(a, B), dotProduct(a, C), dotProduct(a, N)};
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

// -------------PBRT Substitution

static inline float Clamp(float x, float lo, float hi)
{
    return clamp(lo, hi, x);
}

static inline float Erf(float x)
{
    // constants
    float a1 = 0.254829592f;
    float a2 = -0.284496736f;
    float a3 = 1.421413741f;
    float a4 = -1.453152027f;
    float a5 = 1.061405429f;
    float p = 0.3275911f;

    // Save the sign of x
    int sign = 1;
    if (x < 0)
        sign = -1;
    x = std::abs(x);

    // A&S formula 7.1.26
    float t = 1 / (1 + p * x);
    float y =
        1 -
        (((((a5 * t + a4) * t) + a3) * t + a2) * t + a1) * t * std::exp(-x * x);

    return sign * y;
}

static inline float ErfInv(float x)
{
    float w, p;
    x = Clamp(x, -.99999f, .99999f);
    w = -std::log((1 - x) * (1 + x));
    if (w < 5)
    {
        w = w - 2.5f;
        p = 2.81022636e-08f;
        p = 3.43273939e-07f + p * w;
        p = -3.5233877e-06f + p * w;
        p = -4.39150654e-06f + p * w;
        p = 0.00021858087f + p * w;
        p = -0.00125372503f + p * w;
        p = -0.00417768164f + p * w;
        p = 0.246640727f + p * w;
        p = 1.50140941f + p * w;
    }
    else
    {
        w = std::sqrt(w) - 3;
        p = -0.000200214257f;
        p = 0.000100950558f + p * w;
        p = 0.00134934322f + p * w;
        p = -0.00367342844f + p * w;
        p = 0.00573950773f + p * w;
        p = -0.0076224613f + p * w;
        p = 0.00943887047f + p * w;
        p = 1.00167406f + p * w;
        p = 2.83297682f + p * w;
    }
    return p * x;
}

static inline float SinPhi(Vector3f v)
{
    return v.y / std::sqrt(v.x * v.x + v.y * v.y);
}

static inline float Sin2Phi(Vector3f v)
{
    return (v.y * v.y) / (v.x * v.x + v.y * v.y);
}

static inline float CosPhi(Vector3f v)
{
    return v.x / std::sqrt(v.x * v.x + v.y * v.y);
}

static inline float Cos2Phi(Vector3f v)
{
    return (v.x * v.x) / (v.x * v.x + v.y * v.y);
}

static inline float TanPhi(Vector3f v)
{
    return v.y / v.x;
}

static inline float Tan2Phi(Vector3f v)
{
    return (v.y * v.y) / (v.x * v.x);
}

static inline float SinTheta(Vector3f v)
{
    return std::sqrt(v.x * v.x + v.y * v.y) / v.norm();
}

static inline float Sin2Theta(Vector3f v)
{
    return (v.x * v.x + v.y * v.y) / (v.x * v.x + v.y * v.y + v.z * v.z);
}

static inline float CosTheta(Vector3f v)
{
    return v.z / v.norm();
}

static inline float AbsCosTheta(Vector3f v)
{
    return std::abs(CosTheta(v));
}

static inline float Cos2Theta(Vector3f v)
{
    return (v.z * v.z) / (v.x * v.x + v.y * v.y + v.z * v.z);
}

static inline float TanTheta(Vector3f v)
{
    return std::sqrt(v.x * v.x + v.y * v.y) / v.z;
}

static inline float Tan2Theta(Vector3f v)
{
    return (v.x * v.x + v.y * v.y) / (v.z * v.z);
}

static inline Vector3f SphericalDirection(float sinTheta, float cosTheta, float phi)
{
    return Vector3f(sinTheta * std::cos(phi), sinTheta * std::sin(phi), cosTheta);
}

static inline bool SameHemisphere(const Vector3f &w, const Vector3f &wp)
{
    return w.z * wp.z > 0;
}

static inline Vector3f Normalize(const Vector3f &v)
{
    return normalize(v);
}

static inline Vector3f Faceforward(const Vector3f &v, const Vector3f &ref)
{
    return (dotProduct(v, ref) < 0) ? -v : v;
}

using Float = float;

// ======================== End of Utility Functions

// ======================== Beginning of Utility - Microfacet Materials

class MicrofacetDistribution
{
public:
    // Return the differential area, given the half vector wh.
    // In practice, this gives the portion of differential area dA being integrated on
    // that has its normal vector aligned with wh.
    virtual float D(const Vector3f &wh) const = 0;
    // Return the value computed from the shadowing-masking function Lambda.
    // This gives the ratio of invisible masked microfacet area per visible microfacet area.
    virtual float lambda(const Vector3f &w) const = 0;
    // This gives the fraction of microfacets with normal wh that are visible from direction omega.
    // In the usual case where the probability a microfacet is visible is independent of its orientation
    // omega_h, we write this function as G1(omega).
    float G1(const Vector3f &w) const
    {
        return 1.0f / (1.0f + lambda(w));
    }
    // Return the fraction of microfacets in a differential area that are visible from both
    // directions omega_o and omega_i.
    float G(const Vector3f &wo, const Vector3f &wi) const
    {
        return 1.f / (1.f + lambda(wo) + lambda(wi));
    }
    // Sample the half vector (used for both reflection and transmission)
    virtual Vector3f sampleWh(const Vector3f &wo) const = 0;
    // Get the probability density at wh
    float getPdf(const Vector3f &wo, const Vector3f &wh) const;
    // Convert roughness to alpha value.
    // Value close to zero correspond to near-perfect specular reflection.
    static inline float roughnessToAlpha(float roughness)
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
    virtual Vector3f sampleWh(const Vector3f &wo) const override;

private:
    const float alphax, alphay;
};

class TrowbridgeReitzDistribution : public MicrofacetDistribution
{
public:
    TrowbridgeReitzDistribution(float alphax_, float alphay_, bool sampleVis = true) : MicrofacetDistribution(sampleVis), alphax(alphax_), alphay(alphay_) {}
    virtual float D(const Vector3f &wh) const override;
    virtual float lambda(const Vector3f &w) const override;
    virtual Vector3f sampleWh(const Vector3f &wo) const override;

private:
    const float alphax, alphay;
};

// ======================== End of Utility - Microfacet Materials

// ======================== Beginning of Utility - Fresnel Materials

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

// ======================== End of Utility - Fresnel Materials

// ======================== Beginning of Material Section

enum MaterialType
{
    DIFFUSE = (1 << 0),                 // Lambertian diffuse
    SPECULAR_TEST = (1 << 1),           // Perfect specular (FP precision problem still exist)
    DIELETRIC_TEST = (1 << 2),          // Dielectric simulation
    FRESNEL_REFLECTION = (1 << 4),      // Specular fresnel materials
    FRESNEL_TRANSMISSION = (1 << 5),    // Fresnel transmission
    FRESNEL_SPECULAR = (1 << 6),        // Fresnel specular
    MICROFACET_REFLECTION = (1 << 8),   // Microfacet reflection simulation
    MICROFACET_TRANSMISSION = (1 << 9), // Microfacet transmission simulation
    MICROFACET_SPECULAR = (1 << 10)     // Microfacet specular simulation
};

class Material
{
public:
    MaterialType m_type; // Type of the material
    // Vector3f m_color;
    Vector3f m_emission;            // Emission for light sources
    Vector3f Kd;                    // The spectrum of the object for diffuse
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
        float F = fresnel->eval(dotProduct(wo, N))[0];
        if (get_random_float() < F)
            return reflected;
        else
            return refracted;
        break;
    }
    case MICROFACET_REFLECTION:
    {
        if (mfDist == nullptr)
            throw std::runtime_error("Invalid Microfacet BRDF: No specified distribution.");

        if (std::abs(dotProduct(wo, N)) < EPSILON)
            return {0.0f};
        Vector3f wh = toWorld(mfDist->sampleWh(toLocal(wo, N)), N);
        if (dotProduct(wo, wh) < 0)
            return {0.0f}; // Should be rare
        return reflect(wo, wh);
        break;
    }
    case MICROFACET_TRANSMISSION:
    {
        if (mfDist == nullptr)
            throw std::runtime_error("Invalid Microfacet BRDF: No specified distribution.");

        if (std::abs(dotProduct(wo, N)) < EPSILON)
            return {0.0f};
        Vector3f wh = toWorld(mfDist->sampleWh(toLocal(wo, N)), N);
        if (std::abs(dotProduct(wo, wh)) < EPSILON)
            return {0.0f};

        return refract(wo, wh, eta[0]);
        break;
    }
    case MICROFACET_SPECULAR:
    {
        if (mfDist == nullptr)
            throw std::runtime_error("Invalid Microfacet BRDF: No specified distribution.");
        
        if (std::abs(dotProduct(wo, N)) < EPSILON)
            return {0.0f};
        Vector3f wh = toWorld(mfDist->sampleWh(toLocal(wo, N)), N);
        float F = fresnel->eval(dotProduct(wo, wh))[0];
        if (get_random_float() < F)
            return reflect(wo, wh);
        else
            return refract(wo, wh, eta[0]);

        break;
    }
    }
    return {0.0f};
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
    case MICROFACET_REFLECTION:
    {
        if (mfDist == nullptr)
            throw std::runtime_error("Invalid Microfacet BRDF: No specified distribution.");

        Vector3f woLocal = toLocal(wo, N);
        Vector3f wiLocal = toLocal(wi, N);

        if (!SameHemisphere(woLocal, wiLocal))
            return 0;
        Vector3f whLocal = Normalize(woLocal + wiLocal);
        return mfDist->getPdf(woLocal, whLocal) / (4 * dotProduct(woLocal, whLocal));
        break;
    }
    case MICROFACET_TRANSMISSION:
    {
        if (mfDist == nullptr)
            throw std::runtime_error("Invalid Microfacet BRDF: No specified distribution.");

        Vector3f woLocal = toLocal(wo, N);
        Vector3f wiLocal = toLocal(wi, N);
        if (SameHemisphere(woLocal, wiLocal))
            return 0;

        // Compute $\wh$ from $\wo$ and $\wi$ for microfacet transmission
        float eta = (CosTheta(woLocal) > 0) ? (this->eta[0]) : (1 / this->eta[0]);
        Vector3f whLocal = Normalize(woLocal + wiLocal * eta);

        if (dotProduct(woLocal, whLocal) * dotProduct(wiLocal, whLocal) > 0)
            return 0;

        // Compute change of variables _dwh\_dwi_ for microfacet transmission
        float sqrtDenom = dotProduct(woLocal, whLocal) + eta * dotProduct(wiLocal, whLocal);
        float dwh_dwi = std::abs((eta * eta * dotProduct(wiLocal, whLocal)) / (sqrtDenom * sqrtDenom));
        return mfDist->getPdf(woLocal, whLocal) * dwh_dwi;
        break;
    }

    case MICROFACET_SPECULAR:
    {
        if (mfDist == nullptr)
            throw std::runtime_error("Invalid Microfacet BRDF: No specified distribution.");

        float pdf = 0.0f;
        do
        {
            Vector3f woLocal = toLocal(wo, N);
            Vector3f wiLocal = toLocal(wi, N);

            if (!SameHemisphere(woLocal, wiLocal))
                break;

            Vector3f whLocal = Normalize(woLocal + wiLocal);
            pdf += mfDist->getPdf(woLocal, whLocal) / (4 * dotProduct(woLocal, whLocal));
        } while (false);
        do
        {

            Vector3f woLocal = toLocal(wo, N);
            Vector3f wiLocal = toLocal(wi, N);
            if (SameHemisphere(woLocal, wiLocal))
                break;

            // Compute $\wh$ from $\wo$ and $\wi$ for microfacet transmission
            float eta = (CosTheta(woLocal) > 0) ? (this->eta[0]) : (1 / this->eta[0]);
            Vector3f whLocal = Normalize(woLocal + wiLocal * eta);

            if (dotProduct(woLocal, whLocal) * dotProduct(wiLocal, whLocal) > 0)
                break;

            // Compute change of variables _dwh\_dwi_ for microfacet transmission
            float sqrtDenom = dotProduct(woLocal, whLocal) + eta * dotProduct(wiLocal, whLocal);
            float dwh_dwi = std::abs((eta * eta * dotProduct(wiLocal, whLocal)) / (sqrtDenom * sqrtDenom));
            pdf += mfDist->getPdf(woLocal, whLocal) * dwh_dwi;
        } while (false);

        return pdf;
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
    case MICROFACET_REFLECTION:
    {
        if (mfDist == nullptr)
            throw std::runtime_error("Invalid Microfacet BRDF: No specified distribution.");

        Vector3f woLocal = toLocal(wo, N);
        Vector3f wiLocal = toLocal(wi, N);
        Float cosThetaO = AbsCosTheta(woLocal), cosThetaI = AbsCosTheta(wiLocal);
        Vector3f whLocal = wiLocal + woLocal;
        // Handle degenerate cases for microfacet reflection
        if (cosThetaI == 0 || cosThetaO == 0)
            return {0};
        if (whLocal.x == 0 && whLocal.y == 0 && whLocal.z == 0)
            return {0};
        whLocal = Normalize(whLocal);
        // For the Fresnel call, make sure that wh is in the same hemisphere
        // as the surface normal, so that TIR is handled correctly.
        Vector3f F = fresnel->eval(dotProduct(wiLocal, Faceforward(whLocal, Vector3f(0, 0, 1))));
        return Krefl * mfDist->D(whLocal) * mfDist->G(woLocal, wiLocal) * F /
               (4 * cosThetaI * cosThetaO);
        break;
    }
    case MICROFACET_TRANSMISSION:
    {
        if (mfDist == nullptr)
            throw std::runtime_error("Invalid Microfacet BRDF: No specified distribution.");

        Vector3f woLocal = toLocal(wo, N);
        Vector3f wiLocal = toLocal(wi, N);
        if (SameHemisphere(woLocal, wiLocal))
            return {0}; // transmission only

        float cosThetaO = CosTheta(woLocal);
        float cosThetaI = CosTheta(wiLocal);
        if (cosThetaI == 0 || cosThetaO == 0)
            return {0};

        // Compute $\wh$ from $\wo$ and $\wi$ for microfacet transmission
        float eta = (CosTheta(woLocal) > 0) ? (this->eta[0]) : (1 / this->eta[0]);
        Vector3f whLocal = Normalize(woLocal + wiLocal * eta);
        if (whLocal.z < 0)
            whLocal = -whLocal;

        // Same side?
        if (dotProduct(woLocal, whLocal) * dotProduct(wiLocal, whLocal) > 0)
            return {0};

        Vector3f F = fresnel->eval(dotProduct(woLocal, whLocal));

        float sqrtDenom = dotProduct(woLocal, whLocal) + eta * dotProduct(wiLocal, whLocal);
        float factor = 1.00f / eta;

        return Ktrans * (Vector3f{1.0f} - F) *
               std::abs(mfDist->D(whLocal) * mfDist->G(woLocal, wiLocal) * eta * eta *
                        std::abs(dotProduct(wiLocal, whLocal)) * std::abs(dotProduct(woLocal, whLocal)) * factor * factor /
                        (cosThetaI * cosThetaO * sqrtDenom * sqrtDenom));

        break;
    }
    case MICROFACET_SPECULAR:
    {
        if (mfDist == nullptr)
            throw std::runtime_error("Invalid Microfacet BRDF: No specified distribution.");
        // TODO: Add MICROFACET_SPECULAR
        Vector3f result(0.0f);
        {
            Vector3f woLocal = toLocal(wo, N);
            Vector3f wiLocal = toLocal(wi, N);
            Float cosThetaO = AbsCosTheta(woLocal), cosThetaI = AbsCosTheta(wiLocal);
            Vector3f whLocal = wiLocal + woLocal;
            // Handle degenerate cases for microfacet reflection
            if (cosThetaI == 0 || cosThetaO == 0)
                break;
            if (whLocal.x == 0 && whLocal.y == 0 && whLocal.z == 0)
                break;
            whLocal = Normalize(whLocal);
            // For the Fresnel call, make sure that wh is in the same hemisphere
            // as the surface normal, so that TIR is handled correctly.
            Vector3f F = fresnel->eval(dotProduct(wiLocal, Faceforward(whLocal, Vector3f(0, 0, 1))));
            result += Krefl * mfDist->D(whLocal) * mfDist->G(woLocal, wiLocal) * F /
                      (4 * cosThetaI * cosThetaO);
        }
        {

            Vector3f woLocal = toLocal(wo, N);
            Vector3f wiLocal = toLocal(wi, N);
            if (SameHemisphere(woLocal, wiLocal))
                break; // transmission only

            float cosThetaO = CosTheta(woLocal);
            float cosThetaI = CosTheta(wiLocal);
            if (cosThetaI == 0 || cosThetaO == 0)
                break;

            // Compute $\wh$ from $\wo$ and $\wi$ for microfacet transmission
            float eta = (CosTheta(woLocal) > 0) ? (this->eta[0]) : (1 / this->eta[0]);
            Vector3f whLocal = Normalize(woLocal + wiLocal * eta);
            if (whLocal.z < 0)
                whLocal = -whLocal;

            // Same side?
            if (dotProduct(woLocal, whLocal) * dotProduct(wiLocal, whLocal) > 0)
                break;

            Vector3f F = fresnel->eval(dotProduct(woLocal, whLocal));

            float sqrtDenom = dotProduct(woLocal, whLocal) + eta * dotProduct(wiLocal, whLocal);
            float factor = 1.00f / eta;

            result += Ktrans * (Vector3f{1.0f} - F) *
                      std::abs(mfDist->D(whLocal) * mfDist->G(woLocal, wiLocal) * eta * eta *
                               std::abs(dotProduct(wiLocal, whLocal)) * std::abs(dotProduct(woLocal, whLocal)) * factor * factor /
                               (cosThetaI * cosThetaO * sqrtDenom * sqrtDenom));
        }

        return result.regularized();
        break;
    }
    }
    return Vector3f::zero();
}

#endif // RAYTRACING_MATERIAL_H
