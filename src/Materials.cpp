#include <cornelis/Materials.hpp>
namespace cornelis {
namespace models {
auto distributionGTR3p2(float cos_theta_H, float alpha) -> float {
    float alpha2 = alpha * alpha;
    auto cos_theta_H2 = cos_theta_H * cos_theta_H;

    // These are arbitrarly named terms. They serve just to partition the expression.
    float A = (alpha2 + alpha) / (2.0f * Pi);
    float B = 1.0f / std::pow(1.0f + (alpha2 - 1.0f) * cos_theta_H2, 1.5f);
    return A * B;
}

auto lambdaTR(float tan_theta, float alpha) -> float {
    if (std::isinf(tan_theta))
        return 0.0f;
    return (-1.0f + sqrtf(1.0f + alpha * alpha * tan_theta * tan_theta)) * 0.5f;
}

auto shadowMaskingTR(float tan_theta_i, float tan_theta_o, float alpha) -> float {
    return 1.0f / (1.0f + lambdaTR(tan_theta_i, alpha) + lambdaTR(tan_theta_o, alpha));
}

auto schlick(float cos_theta, float refidx1, float refidx2) -> float {
    float R0 = (refidx1 - refidx2) / (refidx1 + refidx2);
    R0 *= R0;
    return R0 + (1.0f - R0) * std::pow(1.0f - cos_theta, 5.0f);
}
} // namespace models
} // namespace cornelis