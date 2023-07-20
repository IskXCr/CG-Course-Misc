/*
    ========= From the author of this file =========
    The author of Material.cpp file does not hold the copyright of the
    following code, though part of it is written by the author.
    Redistribution and use in source and binary forms of the part other
    than sampling (which is from pbrt source code) are permitted without
    any conditions. For the sampling part, please read the disclaimer below.

    ========= From the author of pbrt =========

    pbrt source code is Copyright(c) 1998-2016
                        Matt Pharr, Greg Humphreys, and Wenzel Jakob.

    Code to sample Beckmann distribution and Trobridge-Reitz Distribution
    is part of pbrt.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

    - Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

    - Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
    IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
    TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
    PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 */

#include "Material.hpp"

// ================== Beginning of Fresnel Reflection/Transmission Implementation ==================

Vector3f FresnelConductor::eval(float cosThetaI) const
{
    return frConductor(cosThetaI, eta, k);
}

Vector3f FresnelDielectric::eval(float cosThetaI) const
{
    return Vector3f(frDielectric(cosThetaI, eta[0]));
}

// ===================== End of Fresnel Reflection/Transmission Implementation =====================

// ================== Beginning of Quick PBRT Substituion ==================

#define CHECK(x) \
    if (not(x))  \
    throw std::runtime_error("CHECK failed from pbrt source code.")

#define DCHECK(x) \
    if (not(x))   \
    throw std::runtime_error("DCHECK failed from pbrt source code.")

using Float = float;

static inline float Clamp(float x, float lo, float hi)
{
    return clamp(lo, hi, x);
}

static inline float Erf(float x)
{   
    // constants
    Float a1 = 0.254829592f;
    Float a2 = -0.284496736f;
    Float a3 = 1.421413741f;
    Float a4 = -1.453152027f;
    Float a5 = 1.061405429f;
    Float p = 0.3275911f;

    // Save the sign of x
    int sign = 1;
    if (x < 0) sign = -1;
    x = std::abs(x);

    // A&S formula 7.1.26
    Float t = 1 / (1 + p * x);
    Float y =
        1 -
        (((((a5 * t + a4) * t) + a3) * t + a2) * t + a1) * t * std::exp(-x * x);

    return sign * y;
}

static inline float ErfInv(float x)
{
    Float w, p;
    x = Clamp(x, -.99999f, .99999f);
    w = -std::log((1 - x) * (1 + x));
    if (w < 5) {
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
    } else {
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

static inline float CosPhi(Vector3f v)
{
    return v.x / std::sqrt(v.x * v.x + v.y * v.y);
}

static inline float SinTheta(Vector3f v)
{
    return std::sqrt(v.x * v.x + v.y * v.y) / v.norm();
}

static inline float CosTheta(Vector3f v)
{
    return v.z / v.norm();
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

// ================== End of Quick PBRT Substituion ==================

Vector3f MicrofacetDistribution::partialFr(const Vector3f &wo, const Vector3f &wi) const
{
    float cosThetaO = std::fabs(wo.z / std::sqrt(wo.x * wo.x + wo.y * wo.y));
    float cosThetaI = std::fabs(wi.z / std::sqrt(wi.x * wi.x + wi.y * wi.y));
    Vector3f wh = (wi + wo).normalized();
    if (std::fabs(cosThetaI) < EPSILON || std::fabs(cosThetaO) < EPSILON)
        return Vector3f(0.f);
    if (std::fabs(wh.x) < EPSILON && std::fabs(wh.x) < EPSILON && std::fabs(wh.x) < EPSILON)
        return Vector3f(0.f);
    return G(wo, wi) * D(wh) / (4 * cosThetaI * cosThetaO);
}

float MicrofacetDistribution::getPdf(const Vector3f &wo, const Vector3f &wh) const
{
    float absCosTheta = std::fabs(wh.z / std::sqrt(wh.x * wh.x + wh.y * wh.y));
    if (sampleVisibleArea)
    {
        return D(wh) * G1(wo) * std::fabs(dotProduct(wo, wh)) / absCosTheta;
    }
    else
    {
        return D(wh) * absCosTheta;
    }
}

float BeckmannDistribution::D(const Vector3f &wh) const
{
    float hyp = std::sqrt(wh.x * wh.x + wh.y * wh.y);
    float tanTheta = hyp / wh.z;
    float tan2Theta = tanTheta * tanTheta;
    if (std::isinf(tan2Theta))
        return 0.f;

    float cosTheta = wh.z / wh.norm();
    float cos2Theta = cosTheta * cosTheta;
    float cos4Theta = cos2Theta * cos2Theta;
    float cosPhi = wh.x / hyp;
    float cos2Phi = cosPhi * cosPhi;
    float sinPhi = wh.y / hyp;
    float sin2Phi = sinPhi * sinPhi;

    return std::exp(-tan2Theta * (cos2Phi / (alphax * alphax) + sin2Phi / (alphay * alphay))) / (M_PI * alphax * alphay * cos4Theta);
}

float BeckmannDistribution::lambda(const Vector3f &w) const
{
    float hyp = std::sqrt(w.x * w.x + w.y * w.y);
    float tanTheta = hyp / w.z;
    float absTanTheta = std::abs(tanTheta);
    if (std::isinf(absTanTheta))
        return 0.f;
    float cosPhi = w.x / hyp;
    float cos2Phi = cosPhi * cosPhi;
    float sinPhi = w.y / hyp;
    float sin2Phi = sinPhi * sinPhi;
    float alpha = std::sqrt(cos2Phi * alphax * alphax + sin2Phi * alphay * alphay);
    float a = 1 / (alpha * absTanTheta);
    if (a >= 1.6f)
        return 0.f;
    return (1 - 1.259f * a + 0.396f * a * a) / (3.535f * a + 2.181f * a * a);
}

// from pbrt
static void BeckmannSample11(float cosThetaI, float U1, float U2, float *slope_x, float *slope_y)
{
    /* Special case (normal incidence) */
    if (cosThetaI > .9999)
    {
        float r = std::sqrt(-std::log(1.0f - U1));
        float sinPhi = std::sin(2 * M_PI * U2);
        float cosPhi = std::cos(2 * M_PI * U2);
        *slope_x = r * cosPhi;
        *slope_y = r * sinPhi;
        return;
    }

    /* The original inversion routine from the paper contained
       discontinuities, which causes issues for QMC integration
       and techniques like Kelemen-style MLT. The following code
       performs a numerical inversion with better behavior */
    float sinThetaI =
        std::sqrt(std::max((float)0, (float)1 - cosThetaI * cosThetaI));
    float tanThetaI = sinThetaI / cosThetaI;
    float cotThetaI = 1 / tanThetaI;

    /* Search interval -- everything is parameterized
       in the Erf() domain */
    float a = -1, c = Erf(cotThetaI);
    float sample_x = std::max(U1, (float)1e-6f);

    /* Start with a good initial guess */
    // float b = (1-sample_x) * a + sample_x * c;

    /* We can do better (inverse of an approximation computed in
     * Mathematica) */
    float thetaI = std::acos(cosThetaI);
    float fit = 1 + thetaI * (-0.876f + thetaI * (0.4265f - 0.0594f * thetaI));
    float b = c - (1 + c) * std::pow(1 - sample_x, fit);

    /* Normalization factor for the CDF */
    static const float SQRT_PI_INV = 1.f / std::sqrt(M_PI);
    float normalization =
        1 /
        (1 + c + SQRT_PI_INV * tanThetaI * std::exp(-cotThetaI * cotThetaI));

    int it = 0;
    while (++it < 10)
    {
        /* Bisection criterion -- the oddly-looking
           Boolean expression are intentional to check
           for NaNs at little additional cost */
        if (!(b >= a && b <= c))
            b = 0.5f * (a + c);

        /* Evaluate the CDF and its derivative
           (i.e. the density function) */
        float invErf = ErfInv(b);
        float value =
            normalization *
                (1 + b + SQRT_PI_INV * tanThetaI * std::exp(-invErf * invErf)) -
            sample_x;
        float derivative = normalization * (1 - invErf * tanThetaI);

        if (std::abs(value) < 1e-5f)
            break;

        /* Update bisection intervals */
        if (value > 0)
            c = b;
        else
            a = b;

        b -= value / derivative;
    }

    /* Now convert back into a slope value */
    *slope_x = ErfInv(b);

    /* Simulate Y component */
    *slope_y = ErfInv(2.0f * std::max(U2, (float)1e-6f) - 1.0f);

    CHECK(!std::isinf(*slope_x));
    CHECK(!std::isnan(*slope_x));
    CHECK(!std::isinf(*slope_y));
    CHECK(!std::isnan(*slope_y));
}

// from pbrt
static Vector3f BeckmannSample(const Vector3f &wo, float alphax, float alphay, float U1, float U2)
{
    // TODO
    Vector3f wiStretched = normalize(Vector3f(alphax * wo.x, alphay * wo.y, wo.z));

    // 2. simulate P22_{wi}(x_slope, y_slope, 1, 1)
    float slope_x, slope_y;
    BeckmannSample11(CosTheta(wiStretched), U1, U2, &slope_x, &slope_y);

    // 3. rotate
    float tmp = CosPhi(wiStretched) * slope_x - SinPhi(wiStretched) * slope_y;
    slope_y = SinPhi(wiStretched) * slope_x + CosPhi(wiStretched) * slope_y;
    slope_x = tmp;

    // 4. unstretch
    slope_x = alphax * slope_x;
    slope_y = alphay * slope_y;

    // 5. compute normal
    return normalize(Vector3f(-slope_x, -slope_y, 1.f));
}

// from pbrt
Vector3f BeckmannDistribution::sample(const Vector3f &wo) const
{
    std::array<float, 2> u{0.5, 0.5};
    Vector3f wh;
    if (!sampleVisibleArea)
    {
        // Sample full distribution of normals for Beckmann distribution

        // Compute $\tan^2 \theta$ and $\phi$ for Beckmann distribution sample
        Float tan2Theta, phi;
        if (alphax == alphay)
        {
            Float logSample = std::log(1 - u[0]);
            DCHECK(!std::isinf(logSample));
            tan2Theta = -alphax * alphax * logSample;
            phi = u[1] * 2 * M_PI;
        }
        else
        {
            // Compute _tan2Theta_ and _phi_ for anisotropic Beckmann
            // distribution
            Float logSample = std::log(1 - u[0]);
            DCHECK(!std::isinf(logSample));
            phi = std::atan(alphay / alphax *
                            std::tan(2 * M_PI * u[1] + 0.5f * M_PI));
            if (u[1] > 0.5f)
                phi += M_PI;
            Float sinPhi = std::sin(phi), cosPhi = std::cos(phi);
            Float alphax2 = alphax * alphax, alphay2 = alphay * alphay;
            tan2Theta = -logSample /
                        (cosPhi * cosPhi / alphax2 + sinPhi * sinPhi / alphay2);
        }

        // Map sampled Beckmann angles to normal direction _wh_
        Float cosTheta = 1 / std::sqrt(1 + tan2Theta);
        Float sinTheta = std::sqrt(std::max((Float)0, 1 - cosTheta * cosTheta));
        wh = SphericalDirection(sinTheta, cosTheta, phi);
        if (!SameHemisphere(wo, wh))
            wh = -wh;
    }
    else
    {
        // Sample visible area of normals for Beckmann distribution
        bool flip = wo.z < 0;
        wh = BeckmannSample(flip ? -wo : wo, alphax, alphay, u[0], u[1]);
        if (flip)
            wh = -wh;
    }
    return reflect(wo, wh);
}

float TrobridgeReitzDistribution::D(const Vector3f &wh) const
{
    float hyp = std::sqrt(wh.x * wh.x + wh.y * wh.y);
    float tanTheta = hyp / wh.z;
    float tan2Theta = tanTheta * tanTheta;
    if (std::isinf(tan2Theta))
        return 0.f;

    float cosTheta = wh.z / wh.norm();
    float cos2Theta = cosTheta * cosTheta;
    float cos4Theta = cos2Theta * cos2Theta;
    float cosPhi = wh.x / hyp;
    float cos2Phi = cosPhi * cosPhi;
    float sinPhi = wh.y / hyp;
    float sin2Phi = sinPhi * sinPhi;
    float e = (cos2Phi / (alphax * alphax) + sin2Phi / (alphay + alphay)) * tan2Theta;

    return 1 / (M_PI * alphax * alphay * cos4Theta * (1 + e) * (1 + e));
}

float TrobridgeReitzDistribution::lambda(const Vector3f &w) const
{
    float hyp = std::sqrt(w.x * w.x + w.y * w.y);
    float tanTheta = hyp / w.z;
    float absTanTheta = std::abs(tanTheta);
    if (std::isinf(absTanTheta))
        return 0.f;

    float cosPhi = w.x / hyp;
    float cos2Phi = cosPhi * cosPhi;
    float sinPhi = w.y / hyp;
    float sin2Phi = sinPhi * sinPhi;
    float alpha = std::sqrt(cos2Phi * alphax * alphax + sin2Phi * alphay * alphay);
    float alpha2Tan2Theta = (alpha * absTanTheta) * (alpha * absTanTheta);

    return (-1.f + std::sqrt(1.f + alpha2Tan2Theta)) / 2.f;
}

// from pbrt
static void TrowbridgeReitzSample11(Float cosTheta, Float U1, Float U2, Float *slope_x, Float *slope_y)
{
    // special case (normal incidence)
    if (cosTheta > .9999)
    {
        Float r = sqrt(U1 / (1 - U1));
        Float phi = 6.28318530718 * U2;
        *slope_x = r * cos(phi);
        *slope_y = r * sin(phi);
        return;
    }

    Float sinTheta =
        std::sqrt(std::max((Float)0, (Float)1 - cosTheta * cosTheta));
    Float tanTheta = sinTheta / cosTheta;
    Float a = 1 / tanTheta;
    Float G1 = 2 / (1 + std::sqrt(1.f + 1.f / (a * a)));

    // sample slope_x
    Float A = 2 * U1 / G1 - 1;
    Float tmp = 1.f / (A * A - 1.f);
    if (tmp > 1e10)
        tmp = 1e10;
    Float B = tanTheta;
    Float D = std::sqrt(
        std::max(Float(B * B * tmp * tmp - (A * A - B * B) * tmp), Float(0)));
    Float slope_x_1 = B * tmp - D;
    Float slope_x_2 = B * tmp + D;
    *slope_x = (A < 0 || slope_x_2 > 1.f / tanTheta) ? slope_x_1 : slope_x_2;

    // sample slope_y
    Float S;
    if (U2 > 0.5f)
    {
        S = 1.f;
        U2 = 2.f * (U2 - .5f);
    }
    else
    {
        S = -1.f;
        U2 = 2.f * (.5f - U2);
    }
    Float z =
        (U2 * (U2 * (U2 * 0.27385f - 0.73369f) + 0.46341f)) /
        (U2 * (U2 * (U2 * 0.093073f + 0.309420f) - 1.000000f) + 0.597999f);
    *slope_y = S * z * std::sqrt(1.f + *slope_x * *slope_x);

    CHECK(!std::isinf(*slope_y));
    CHECK(!std::isnan(*slope_y));
}

// from pbrt
static Vector3f TrowbridgeReitzSample(const Vector3f &wi, Float alpha_x, Float alpha_y, Float U1, Float U2)
{
    // 1. stretch wi
    Vector3f wiStretched =
        Normalize(Vector3f(alpha_x * wi.x, alpha_y * wi.y, wi.z));

    // 2. simulate P22_{wi}(x_slope, y_slope, 1, 1)
    Float slope_x, slope_y;
    TrowbridgeReitzSample11(CosTheta(wiStretched), U1, U2, &slope_x, &slope_y);

    // 3. rotate
    Float tmp = CosPhi(wiStretched) * slope_x - SinPhi(wiStretched) * slope_y;
    slope_y = SinPhi(wiStretched) * slope_x + CosPhi(wiStretched) * slope_y;
    slope_x = tmp;

    // 4. unstretch
    slope_x = alpha_x * slope_x;
    slope_y = alpha_y * slope_y;

    // 5. compute normal
    return Normalize(Vector3f(-slope_x, -slope_y, 1.));
}

// from pbrt
Vector3f TrobridgeReitzDistribution::sample(const Vector3f &wo) const
{
    std::array<float, 2> u{0.5, 0.5};
    // TODO
    Vector3f wh;
    if (!sampleVisibleArea)
    {
        Float cosTheta = 0, phi = (2 * M_PI) * u[1];
        if (alphax == alphay)
        {
            Float tanTheta2 = alphax * alphax * u[0] / (1.0f - u[0]);
            cosTheta = 1 / std::sqrt(1 + tanTheta2);
        }
        else
        {
            phi =
                std::atan(alphay / alphax * std::tan(2 * M_PI * u[1] + .5f * M_PI));
            if (u[1] > .5f)
                phi += M_PI;
            Float sinPhi = std::sin(phi), cosPhi = std::cos(phi);
            const Float alphax2 = alphax * alphax, alphay2 = alphay * alphay;
            const Float alpha2 =
                1 / (cosPhi * cosPhi / alphax2 + sinPhi * sinPhi / alphay2);
            Float tanTheta2 = alpha2 * u[0] / (1 - u[0]);
            cosTheta = 1 / std::sqrt(1 + tanTheta2);
        }
        Float sinTheta =
            std::sqrt(std::max((Float)0., (Float)1. - cosTheta * cosTheta));
        wh = SphericalDirection(sinTheta, cosTheta, phi);
        if (!SameHemisphere(wo, wh))
            wh = -wh;
    }
    else
    {
        bool flip = wo.z < 0;
        wh = TrowbridgeReitzSample(flip ? -wo : wo, alphax, alphay, u[0], u[1]);
        if (flip)
            wh = -wh;
    }

    return reflect(wo, wh);
}
