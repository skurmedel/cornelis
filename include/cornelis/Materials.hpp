#pragma once

#include <cornelis/Color.hpp>
#include <cornelis/Math.hpp>

namespace cornelis {
/**
 * Describes the "Bi-directional Scattering Distribution Function". This function describes the
 * amount of light scattered for a certain pair of directions.
 */
struct BSDF {
    virtual ~BSDF() = default;

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
     * For this function to be physically plausible the following must hold:
     *  - reciprocity so for a BSDF B we should have B(u, v) = B(v, u)
     *  - energy conserving, i.e the integral of this function over the sphere is at most 1.
     *  - np negative values
     */
    virtual auto operator()(float3 const &wi, float3 const &wo) const noexcept -> RGB = 0;

    /**
     * Query the probability density function for this BSDF. This function should integrate to 1
     * over the sphere (at least in theory.)
     *
     * Returns the probability that light is scattered in the given direction.
     */
    virtual auto pdf(float3 const &wi) const noexcept -> float = 0;

    // TODO: support refracting materials.
};

class LambertBSDF : public BSDF {
  public:
    LambertBSDF(RGB albedo) : albedo_(albedo) {}

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
    StandardMaterial(RGB albedo, RGB emission) : emission_(emission), bsdf_(albedo) {}

    auto bsdf(float3 const &P, float3 const &N) const noexcept -> BSDF const & { return bsdf_; }

    auto emission(float3 const &P) const noexcept -> RGB { return emission_; }

  private:
    RGB emission_;
    LambertBSDF bsdf_;
};
} // namespace cornelis
