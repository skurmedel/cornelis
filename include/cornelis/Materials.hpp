#pragma once

#include <cornelis/Color.hpp>
#include <cornelis/Math.hpp>

namespace cornelis {
// TODO: move this to .cpp
namespace models {
/**
 * Generalized Trowbridge-Rietz microfacet distribution as suggested by Brent Burley in Physically
 * Base Shading at Disney. This is the version with gamma = 1.5f (i.e somewhat fat tail).
 *
 * @param cos_theta_H Cosine of the angle between N and the halfway vector (theta refers to
 * spherical coordinates.)
 * @param alpha   A roughness constant, should be in [0,1].
 */
auto distributionGTR3p2(float cos_theta_H, float alpha) -> float;
/**
 * Gives the lambda function used in the shadowing and masking term for a Trowbridge-Reitz
 * microfacet distribution.
 *
 * @param tan_theta   Tangent of the angle between the normal and the direction being considered.
 * @param alpha   A roughness constant, should be in [0,1].
 */
auto lambdaTR(float tan_theta, float alpha) -> float;
/**
 * @param tan_theta_i Tangent of the angle between normal and incoming direction
 * @param tan_theta_o Tangent of the angle between normal and outgoing direction.
 * @param alpha   See lambdaTR
 */
auto shadowMaskingTR(float theta_i, float theta_o, float alpha) -> float;
/**
 * Computes the Fresnel coefficient using Schlick's approximation.
 * @param cos_theta Cosine of the angle between the normal and the viewer (the outgoing direction
 * usually)
 * @param refidx1 Refractive index at the surface interface.
 * @param refidx2 The other refractive index at the surface interface.
 */
auto schlick(float theta, float refidx1, float refidx2) -> float;
} // namespace models
namespace detail {
/**
 * Gets the cosine of theta in the spherical coordinates formulation.
 *
 * This is just the z component.
 * @param w A direction.
 */
auto inline cosTheta(float3 const &w) -> float { return w(2); }
} // namespace detail
/**
 * Describes the "Bi-directional Reflectance Distribution Function". This function describes the
 * amount of light scattered for a certain pair of directions.
 */
struct BRDF {
    virtual ~BRDF() = default;

    /**
     * wi is the input direction, wo is the output direction. This returns the density of light
     * scattered for these parameters.
     *
     * Note that wo "points toward the viewer", and wi "points toward the light".
     *
     * We assume this function treats wavelengths independently, and so for example red light in
     * won't become green light out. This makes flouroscence impossible to model with this function.
     *
     * Furthermore, it returns the results for all wavelengths at the same time. They are thus in a
     * sense, coupled.
     *
     * Note: this function is usually called f() in the rendering equation, and is what we usually
     * call the BRDF.
     *
     * For this function to be physically plausible the following must hold:
     *  - reciprocity so for a BRDF B we should have B(u, v) = B(v, u)
     *  - energy conserving, i.e the integral of this function over the sphere is at most 1.
     *  - np negative values
     */
    virtual auto operator()(float3 const &wi, float3 const &wo, float3 const &N) const noexcept
        -> RGB = 0;

    /**
     * Query the probability density function for this BRDF. This function should integrate to 1
     * over the sphere (at least in theory.)
     *
     * Returns the probability that light is scattered in the given direction.
     */
    virtual auto pdf(float3 const &wi) const noexcept -> float = 0;

    // TODO: support refracting materials.
};

class GlossyBRDF : public BRDF {
  public:
    /**
     * @param tint  Tint of highlights.
     * @param sigma Roughness parameter, between [0, 1].
     * @param refidx Refractive index.
     */
    GlossyBRDF(RGB tint, float alpha, float refidx = 1.5)
        : tint_(tint), alpha_(alpha), refidx_(refidx) {}

    auto operator()(float3 const &wi, float3 const &wo, float3 const &N) const noexcept
        -> RGB override {
        // TODO: we could simplify and optimise this a lot by basis change.
        // TODO: probably numerically troublesome. Can be rewritten.

        float cos_thetaO = std::max(0.0f, dot(wo, N));
        float cos_thetaI = std::max(0.0f, dot(wi, N));
        // This check is crucial, because if this starts generating NaNs the whole image can
        // end up black or some other strange colour, and it's extremely annoying to find the cause.
        if (isAlmostZero(cos_thetaO) || isAlmostZero(cos_thetaI))
            return RGB::black();

        float3 h = normalize(wi + wo);
        if (isAlmostZero(h(0)) && isAlmostZero(h(1)) && isAlmostZero(h(2)))
            return RGB::black();
        float cos_theta_H = std::max(0.0f, dot(h, N));

        float D = models::distributionGTR3p2(cos_theta_H, alpha_);
        float G = models::shadowMaskingTR(cos_thetaI, cos_thetaO, alpha_);
        float F = models::schlick(cos_theta_H, 1.0f, refidx_);

        return tint_ * (F * D * G / (4.0f * cos_thetaO * cos_thetaI));
    }
    auto pdf(float3 const &wi) const noexcept -> float override {
        // This PDF is wrong and just copied from Lambert, fix when we do Importance Sampling.
        return 1.0f / (4.0f * cornelis::Pi);
    }

    auto tint() const noexcept -> RGB { return tint_; }

  private:
    RGB tint_;
    float alpha_;
    float refidx_;
};

class OrenNayarBRDF : public BRDF {
  public:
    /**
     * @param albedo  The underlying "colour"
     * @param sigma   Roughness parameter in radians.
     */
    OrenNayarBRDF(RGB albedo, float sigma)
        : albedo_(albedo), sigma2_(sigma * sigma),
          a_(1.0f - (sigma2_ / (2.0f * (sigma2_ + 0.333f)))),
          b_(0.45f * sigma2_ / (sigma2_ + 0.09f)) {}

    auto operator()(float3 const &wi, float3 const &wo, float3 const &N) const noexcept
        -> RGB override {
        // TODO: we could simplify and optimise this a lot by basis change.
        // TODO: probably numerically troublesome. Can be rewritten.
        float cosThetaI = wi(2);
        float cosThetaO = wo(2);
        float sinThetaI = sqrtf(1.0f - cosThetaI * cosThetaI);
        float sinThetaO = sqrtf(1.0f - cosThetaO * cosThetaO);
        float phiI = std::acos(wi(0) / sinThetaI);
        float phiO = std::acos(wo(0) / sinThetaO);
        float thetaO = std::acos(cosThetaO);
        float thetaI = std::acos(cosThetaI);
        float alpha = std::max(thetaI, thetaO);
        float beta = std::min(thetaI, thetaO);

        return (albedo_ / cornelis::Pi) *
               (a_ + b_ * std::max(0.0f, cosf(phiI - phiO)) * sin(alpha) * sin(beta));
    }
    auto pdf(float3 const &wi) const noexcept -> float override {
        // This PDF is wrong and just copied from Lambert, fix when we do Importance Sampling.
        return 1.0f / (4.0f * cornelis::Pi);
    }

    auto albedo() const noexcept -> RGB { return albedo_; }

  private:
    RGB albedo_;
    float sigma2_;
    float a_;
    float b_;
};

/**
 * This is a BRDF that approximates a material that has thin glossy layer on top of a diffuse one.
 * 
 * This is suitable for things like a painted surface, wood and so forth. It can probably be abused 
 * to look like most surfaces though.
 */
class LayeredBRDF : public BRDF {
  public:
    /**
     * @param albedo  The underlying "colour"
     * @param sigma   Roughness parameter in radians.
     */
    LayeredBRDF(OrenNayarBRDF diffuseBrdf, GlossyBRDF glossyBrdf, float reflectance)
        : diffuse_(diffuseBrdf), glossy_(glossyBrdf), reflectance_(reflectance) {}

    auto operator()(float3 const &wi, float3 const &wo, float3 const &N) const noexcept
        -> RGB override {
        RGB D_f = diffuse_(wi, wo, N);
        RGB G_f = glossy_(wi, wo, N);
        constexpr auto w_term = [](float3 const &w, float3 const &N) {
            auto N_dot_w = std::max(0.0f, dot(N, w));
            return 1.0f - std::pow(1.0f - 0.5f * N_dot_w, 5.0f);
        };
        // This is not very realistic but at least scales the diffuse at grazing angles.
        return (1.0f - models::schlick(std::max(0.0f, dot(N, wi)), 1.0f, 1.5f)) * D_f + G_f;
    }
    auto pdf(float3 const &wi) const noexcept -> float override {
        // This PDF is wrong and just copied from Lambert, fix when we do Importance Sampling.
        return 1.0f / (4.0f * cornelis::Pi);
    }

  private:
    OrenNayarBRDF diffuse_;
    GlossyBRDF glossy_;
    float reflectance_;
};

class LambertBRDF : public BRDF {
  public:
    LambertBRDF(RGB albedo) : albedo_(albedo) {}

    auto operator()(float3 const &wi, float3 const &wo, float3 const &N) const noexcept
        -> RGB override {
        return albedo_ / cornelis::Pi;
    }
    auto pdf(float3 const &wi) const noexcept -> float override {
        // This is the area of a unit sphere and represents a completely uniform distribution.
        return 1.0f / (4.0f * cornelis::Pi);
    }

  private:
    RGB albedo_;
};

class StandardMaterial {
  public:
    StandardMaterial(RGB albedo, RGB emission)
        : emission_(emission),
          bsdf_(OrenNayarBRDF(albedo, 0.2), GlossyBRDF(RGB(0.8, 0.8, 0.8), 0.05f), 0.3f) {}

    auto brdf(float3 const &P, float3 const &N) const noexcept -> BRDF const & { return bsdf_; }

    auto emission(float3 const &P) const noexcept -> RGB { return emission_; }

  private:
    RGB emission_;
    LayeredBRDF bsdf_;
};
} // namespace cornelis
