#pragma once

#include <cornelis/Color.hpp>
#include <cornelis/Math.hpp>
#include <cornelis/PRNG.hpp> // TODO: get rid of this include.

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
 * Same as distributionGTR3p2 but with gamma = 2 as an exponent. This is called GGX by many.
 */
auto distributionGTR2(float cos_theta_H, float alpha) -> float;
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
     * This kitchen sink function generates a "reflected" direction from the output direction and
     * three random variables (x).
     *
     * It also stores the probability density function value in pdf, and returns the BRDF for the
     * generated direction.
     *
     * Note: pbrt and others calls this sample_f, I think that's a profoundly useless name. This
     * name isn't much better and probably indicates that this function should be of a different
     * form.
     *
     * By default this randomly samples the hemisphere.
     *
     * @param wo  Light out direction (world), commonly called the viewer.
     * @param x   Three sample floats, usually just 2 are needed. The third can be used for choices.
     * @param b   Local basis for the point on the surface.
     * @param wi  Light in direction (world), commonly called the light.
     * @param pdf Probability that this direction was chosen.
     * @return RGB The BRDF (f-value) for these directions.
     */
    virtual auto
    generateDirection(float3 const &wo, float3 x, Basis const &b, float3 &wi, float &pdf) const
        -> RGB {
        wi = randomHemisphere(float2(x(0), x(1)), b);
        pdf = this->pdf(wi, wo, b);
        return (*this)(wi, wo, b.N);
    }

    virtual auto pdf(float3 const &wi, float3 const &wo, Basis const &b) const noexcept -> float {
        return randomHemispherePDF();
    }

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
        float sin_thetaO = sqrtf(1.0f - cos_thetaO * cos_thetaO);
        float cos_thetaI = std::max(0.0f, dot(wi, N));
        float sin_thetaI = sqrtf(1.0f - cos_thetaI * cos_thetaI);
        // This check is crucial, because if this starts generating NaNs the whole image can
        // end up black or some other strange colour, and it's extremely annoying to find the cause.
        if (isAlmostZero(cos_thetaO) || isAlmostZero(cos_thetaI))
            return RGB::black();

        float3 h = normalize(wi + wo);
        if (isAlmostZero(h(0)) && isAlmostZero(h(1)) && isAlmostZero(h(2)))
            return RGB::black();
        float cos_theta_H = std::max(0.0f, dot(h, N));

        float D = models::distributionGTR2(cos_theta_H, alpha_);
        float G = models::shadowMaskingTR(sin_thetaI / cos_thetaI, sin_thetaO / cos_thetaO, alpha_);
        float F = models::schlick(cos_theta_H, 1.0f, refidx_);

        return tint_ * (F * D * G / (4.0f * cos_thetaO * cos_thetaI));
    }

    auto generateDirection(float3 const &wo, float3 x, Basis const &b, float3 &wi, float &pdf) const
        -> RGB override {
        // Todo: move this into a function.
        float alpha2 = alpha_ * alpha_;
        float A = 1.0f - x(1);
        float B = 1.0f + (alpha2 - 1.0f) * x(1);
        float cos_theta_H = sqrtf(A / B);
        float sin_theta_H = sqrt(1.0f - cos_theta_H * cos_theta_H);

        float phih = 2.0f * Pi * x(0);

        float3 h = normalize(sin_theta_H * cos(phih) * b.B + sin_theta_H * sin(phih) * b.T +
                             cos_theta_H * b.N);
        if (dot(h, b.N) < 0.0f)
            return RGB(0.0f, 0.0f, 0.0f);
        wi = normalize(2.0 * dot(wo, h) * h - wo);

        pdf = this->pdf(wi, wo, b);
        return (*this)(wi, wo, b.N);
    }

    auto pdf(float3 const &wi, float3 const &wo, Basis const &b) const noexcept -> float override {
        float3 h = normalize(wi + wo);
        float cos_theta_h = std::max(0.0f, dot(h, b.N));
        if (isAlmostZero(cos_theta_h))
            return 1.0f;
        float D = models::distributionGTR2(cos_theta_h, alpha_);
        float pdfh = D * abs(cos_theta_h);
        float wi_dot_h = dot(wi, h);
        if (isAlmostZero(wi_dot_h))
            return pdfh;
        return pdfh / (4.0f * wi_dot_h);
    }

    auto tint() const noexcept -> RGB { return tint_; }

    auto ior() const noexcept -> float { return refidx_; }

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

    auto albedo() const noexcept -> RGB { return albedo_; }

  private:
    RGB albedo_;
    float sigma2_;
    float a_;
    float b_;
};

/**
 * This is a BRDF that approximates a material that has a thin glossy layer on top of a diffuse one.
 *
 * This is suitable for things like a painted surface, wood and so forth. It can probably be abused
 * to look like most opaque surfaces though.
 */
class LayeredBRDF : public BRDF {
  public:
    /**
     * @param albedo  The underlying "colour"
     * @param sigma   Roughness parameter in radians.
     */
    LayeredBRDF(RGB albedo, RGB glossyTint, float perceptualRoughness, float ior)
        : diffuse_(albedo, diffuseRough(perceptualRoughness)),
          glossy_(glossyTint, glossyRough(perceptualRoughness), ior) {}

    auto operator()(float3 const &wi, float3 const &wo, float3 const &N) const noexcept
        -> RGB override {
        RGB D_f = diffuse_(wi, wo, N);
        RGB G_f = glossy_(wi, wo, N);
        // This is not very realistic but at least scales the diffuse at grazing angles.
        // It is similar to the model of Ashikimin and Shirley, but probably less realistic.
        return (1.0f - models::schlick(std::max(0.0f, dot(N, wi)), 1.0f, glossy_.ior())) * D_f +
               G_f;
    }

    auto pdf(float3 const &wi, float3 const &wo, Basis const &b) const noexcept -> float override {
        /* Since we have chosen between two alternatives, we need to multiply our PDF by the
           probability of the chosen path. Let X be the probability of the generated direction, and
           K the probability of the choice.

           P(X and K) = P(X | K) * P(K) But P(K) = 1/2, so P(X and K) = 0.5 * P(X | K)

           However this is troublesome in our case, since the glossy layer will likely have a low
           pdf when the incident angle is low. This will underestimate the diffuse, and we will get
           greater variance. For this reason, the pdf() function choses a weighted average instead.
          */
        return 0.5f * (diffuse_.pdf(wi, wo, b) + glossy_.pdf(wi, wo, b));
    }

    auto generateDirection(float3 const &wo, float3 x, Basis const &b, float3 &wi, float &pdf) const
        -> RGB override {
        float pdf_chosen = 0.0f;
        RGB brdf;
        if (x(2) < 0.5f) {
            x(2) *= 2.0f; // Readjust x(2) since we have made a choice on it.
            brdf = diffuse_.generateDirection(wo, x, b, wi, pdf_chosen);
        } else {
            x(2) = (x(2) - 0.5f) * 2.0f;
            brdf = glossy_.generateDirection(wo, x, b, wi, pdf_chosen);
        }

        pdf = this->pdf(wi, wo, b);
        return (*this)(wi, wo, b.N);
    }

  private:
    static inline auto glossyRough(float perceptual) -> float {
        // This is a remapping suggested by Brent Burley in the Disney Principled Shader paper.
        return perceptual * perceptual;
    }
    static inline auto diffuseRough(float perceptual) -> float {
        return abs(0.5f * glossyRough(perceptual));
    }

    OrenNayarBRDF diffuse_;
    GlossyBRDF glossy_;
};

class LambertBRDF : public BRDF {
  public:
    LambertBRDF(RGB albedo) : albedo_(albedo) {}

    auto operator()(float3 const &wi, float3 const &wo, float3 const &N) const noexcept
        -> RGB override {
        return albedo_ / cornelis::Pi;
    }
    auto pdf(float3 const &wi, float3 const &wo, Basis const &b) const noexcept -> float override {
        // This is the area of a unit sphere and represents a completely uniform distribution.
        return 1.0f / (4.0f * cornelis::Pi);
    }

  private:
    RGB albedo_;
};

class StandardMaterial {
  public:
    StandardMaterial(
        RGB albedo, RGB emission, RGB reflectionTint, float roughness = 0.1f, float ior = 1.5f)
        : emission_(emission), bsdf_(albedo, reflectionTint, roughness, ior) {}

    auto brdf(float3 const &P, float3 const &N) const noexcept -> BRDF const & { return bsdf_; }

    auto emission(float3 const &P) const noexcept -> RGB { return emission_; }

  private:
    RGB emission_;
    LayeredBRDF bsdf_;
};
} // namespace cornelis
