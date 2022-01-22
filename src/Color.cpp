#include <cornelis/Color.hpp>
namespace cornelis {

auto RGB::operator+=(RGB const &rgb) noexcept -> RGB & {
    for (int i = 0; i < 3; i++) {
        this->operator()(i) += rgb(i);
    }
    return *this;
}

auto RGB::operator/(float scalar) const noexcept -> RGB {
    RGB result;
    for (int i = 0; i < 3; i++) {
        result(i) = this->operator()(i) / scalar;
    }
    return result;
}

auto RGB::operator*=(RGB const &other) noexcept -> RGB & {
    for (int i = 0; i < 3; i++) {
        this->operator()(i) *= other(i);
    }
    return *this;
}

// adapted from GLSL versions at
// https://github.com/skurmedel/shaders/blob/master/glsl/gamma_correct.glsl
/*
    Takes an sRGB gamma corrected triple and linearizes it.
    The input is assumed to be gamma corrected according to the sRGB standard.
    The sRGB gamma function is not just pow(x, 2.2), but instead a function that
    is linear close to black and non-linear for brighter colours.
*/
SRGB srgb_gamma_linearize(SRGB rgb) {
    // Defined by the sRGB standard, as are all the other constants.
    const float a = 0.055f;

    // We use step here to avoid an if clause since those are generally not
    // recommended in a GPU context.
#define SRGB_TRANSFORM_CH(v, x)                                                                    \
    { v = (x <= 0.04045) ? (x / 12.95f) : pow((x + a) / (1 + a), 2.4); }

    SRGB ret = SRGB{0};
    SRGB_TRANSFORM_CH(ret(0), rgb(0));
    SRGB_TRANSFORM_CH(ret(1), rgb(1));
    SRGB_TRANSFORM_CH(ret(2), rgb(2));

#undef SRGB_TRANSFORM_CH

    return ret;
}

/*
    Takes a linear colour triple and gamma corrects it according to sRGB
    standards.
    See the comments for srgb_gamma_linearize for more info.
*/
SRGB srgb_gamma_correct(SRGB rgb) {
    const float a = 0.055f;

#define SRGB_TRANSFORM_CH(v, x)                                                                    \
    { v = (x <= 0.0031308) ? (x * 12.95f) : ((1 + a) * pow(x, 1.0f / 2.4f) - a); }

    SRGB ret = SRGB{0};
    SRGB_TRANSFORM_CH(ret(0), rgb(0));
    SRGB_TRANSFORM_CH(ret(1), rgb(1));
    SRGB_TRANSFORM_CH(ret(2), rgb(2));

#undef SRGB_TRANSFORM_CH

    return ret;
}

auto toSRGB(RGB const &rgb) -> SRGB { return srgb_gamma_correct({rgb(0), rgb(1), rgb(2)}); }
} // namespace cornelis
