#include "brain_inference.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
namespace mnm::reconstruction::combat {
namespace {
long double exponential(long double value) {
#if defined(__i386__) || defined(__x86_64__)
  // Same x87 exp reduction as the original (rounding mode is round-to-nearest).
  long double out;
  asm volatile("fldl2e; fmulp; fld %%st(0); frndint; fxch; fsub %%st(1), "
               "%%st(0); f2xm1; fld1; faddp; fscale; fstp %%st(1)"
               : "=t"(out)
               : "0"(value));
  return out;
#else
  return std::exp(value);
#endif
}
} // namespace
void validateBrainInference(const BrainInferenceModel &m) {
  if (!m.dimension || m.dimension > 128 || m.layers.empty() ||
      m.layers.size() > 64)
    throw std::invalid_argument("owner brain model dimensions");
  for (const auto &l : m.layers) {
    if (l.nodes.size() != m.dimension ||
        l.matrix.size() != m.dimension * m.dimension ||
        !std::isfinite(l.observations) || !std::isfinite(l.outside))
      throw std::invalid_argument("owner brain layer dimensions");
    for (const auto &n : l.nodes) {
      for (float f : n)
        if (!std::isfinite(f))
          throw std::invalid_argument("owner brain nonfinite node");
      if (n[1] <= 0)
        throw std::invalid_argument("owner brain node scale");
    }
    for (float f : l.matrix)
      if (!std::isfinite(f))
        throw std::invalid_argument("owner brain nonfinite matrix");
  }
}
std::vector<float> inferOwnerTask(const BrainInferenceModel &m,
                                  const std::vector<std::int32_t> &features) {
  validateBrainInference(m);
  if (features.size() != m.dimension)
    throw std::invalid_argument("owner brain feature dimensions");
  if (std::count_if(m.layers.begin(), m.layers.end(),
                    [](const auto &l) { return l.observations > 0; }) < 2)
    return std::vector<float>(m.layers.size(), float(1.L / m.layers.size()));
  std::vector<float> scores, values(m.dimension);
  float minimum = 1e10f;
  for (const auto &l : m.layers) {
    for (unsigned i = 0; i < m.dimension; ++i) {
      const auto &n = l.nodes[i];
      const long double scaled =
          (static_cast<long double>(features[i]) - n[0]) / n[1] * 10.L;
      // 59bee0 truncates to a signed64 result, returning its low signed DWORD.
      // Native admission rejects ranges that cannot be interpreted safely.
      if (!std::isfinite(scaled) || scaled <= -0x1p31L || scaled >= 0x1p31L)
        throw std::invalid_argument("owner brain feature conversion bound");
      int bin = int(scaled);
      values[i] = bin < 0     ? -l.outside
                  : bin >= 10 ? l.outside
                              : n[unsigned(bin) + 2];
    }
    volatile float score = 0;
    for (unsigned row = 0; row < m.dimension; ++row)
      for (unsigned column = 0; column < m.dimension; ++column)
        score = float(
            static_cast<long double>(l.matrix[row * m.dimension + column]) *
                values[column] * values[row] +
            score);
    if (!std::isfinite(float(score)))
      throw std::invalid_argument("owner brain energy overflow");
    minimum = std::min(minimum, float(score));
    scores.push_back(float(score));
  }
  long double sum = 0;
  for (auto &score : scores) {
    const long double shifted =
        (static_cast<long double>(score) - minimum) * .5L;
    const long double exp = shifted >= 20.L ? 0 : exponential(-shifted);
    score = float(exp);
    sum += exp;
  }
  if (!std::isfinite(sum) || sum <= 0)
    throw std::invalid_argument("owner brain normalization");
  for (auto &score : scores)
    score = float(static_cast<long double>(score) / sum);
  return scores;
}
float ownerTaskValue(const std::array<float, 3> &p,
                     const std::array<float, 3> &w, int percent) {
  for (float f : p)
    if (!std::isfinite(f) || f < 0 || f > 1)
      throw std::invalid_argument("owner task probability");
  for (float f : w)
    if (!std::isfinite(f))
      throw std::invalid_argument("owner task weight");
  if (percent < 0 || percent > 10000)
    throw std::invalid_argument("owner task percentage");
  // The exact upper cap is pinned by the discovery table and direct test.
  float value =
      float(std::min(.999L, static_cast<long double>(w[2]) * p[2] -
                                static_cast<long double>(w[0]) * p[0]));
  if (percent != 100) {
    // Hexadecimal preserves the pinned float's value even under i386 C++
    // excess-precision constant folding (a decimal .01f cast is insufficient).
    const long double scaled =
        (static_cast<long double>(value) + 1.L) * percent * 0x1.47ae14p-7L -
        1.L;
    value = float(scaled);
    if (scaled >= 1)
      value = .999f;
    if (value < -1)
      value = -1;
  }
  return value;
}
} // namespace mnm::reconstruction::combat
