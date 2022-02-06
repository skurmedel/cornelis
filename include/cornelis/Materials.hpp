#pragma once

#include <cornelis/Color.hpp>
#include <cornelis/Math.hpp>

namespace cornelis {
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
    virtual auto operator()(float3 const &wi, float3 const &wo) const noexcept -> RGB = 0;

    /**
     * Query the probability density function for this BRDF. This function should integrate to 1
     * over the sphere (at least in theory.)
     *
     * Returns the probability that light is scattered in the given direction.
     */
    virtual auto pdf(float3 const &wi) const noexcept -> float = 0;

    // TODO: support refracting materials.
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

    auto operator()(float3 const &wi, float3 const &wo) const noexcept -> RGB override {
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

  private:
    RGB albedo_;
    float sigma2_;
    float a_;
    float b_;
};

class LambertBRDF : public BRDF {
  public:
    LambertBRDF(RGB albedo) : albedo_(albedo) {}

    auto operator()(float3 const &wi, float3 const &wo) const noexcept -> RGB override {
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
    StandardMaterial(RGB albedo, RGB emission) : emission_(emission), bsdf_(albedo, Pi / 6.0f) {}

    auto brdf(float3 const &P, float3 const &N) const noexcept -> BRDF const & { return bsdf_; }

    auto emission(float3 const &P) const noexcept -> RGB { return emission_; }

  private:
    RGB emission_;
    OrenNayarBRDF bsdf_;
};
} // namespace cornelis
