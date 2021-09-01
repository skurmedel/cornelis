#include <array>
#include <xsimd/xsimd.hpp>

#include <cornelis/Linalg.hpp>

#if !(defined(XSIMD_X86_AVX_VERSION_AVAILABLE) || defined(XSIMD_ARM8_64_NEON_VERSION))
#error We require AVX or NEON 64 at the moment, sorry :(
#endif

using batch4f = xsimd::batch<float, 4>;
// using batch8f = xsimd::batch<float, 8>;

namespace {
struct simdMatrix4x4 {
    // TODO: tidy up this dumpster fire.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
    static xsimd::aligned_mode aligned_tag;
#pragma clang diagnostic pop

    simdMatrix4x4(cornelis::float4x4 const &input)
        : rows{batch4f{input.values + 0, aligned_tag},
               batch4f{input.values + 4, aligned_tag},
               batch4f{input.values + 8, aligned_tag},
               batch4f{input.values + 12, aligned_tag}} {}

    auto leftMultiply(batch4f const &x, batch4f &result) const -> void {
        std::array<batch4f, 4> rowsProducts = rows;
        // TODO: we can improve on this greatly if we have float4x4 arranged by columns instead of
        // rows.

        rowsProducts[0] *= x;
        rowsProducts[1] *= x;
        rowsProducts[2] *= x;
        rowsProducts[3] *= x;
        result = xsimd::haddp(rowsProducts.data());
    }

    std::array<batch4f, 4> rows;
};
} // namespace

namespace cornelis {

auto matrixMultiply(float4x4 const &A, float4 const &x, float4 &y) noexcept -> void {
    batch4f x_b;
    x_b.load_aligned(x.values);

    simdMatrix4x4 matrix(A);
    batch4f result;
    matrix.leftMultiply(x_b, result);
    result.store_aligned(y.values);
}

auto transformRays(float4x4 const &A, std::span<ray4> rays) noexcept -> void {
    simdMatrix4x4 matrix(A);

    for (auto &ray : rays) {
        batch4f pos;
        batch4f dir;
        pos.load_aligned(ray.pos.values);
        dir.load_aligned(ray.dir.values);

        matrix.leftMultiply(pos, pos);
        matrix.leftMultiply(dir, dir);
        pos.store_aligned(ray.pos.values);
        dir.store_aligned(ray.dir.values);
    }
 }
} // namespace cornelis
