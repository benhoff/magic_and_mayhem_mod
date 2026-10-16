#pragma once
#include <array>
#include <cstdint>
#include <vector>
namespace mnm::reconstruction::combat {
// Owned numeric model; independent of asset IO and original packed containers.
struct BrainInferenceLayer {
  float observations = 0, outside = 0;
  std::vector<std::array<float, 12>> nodes;
  std::vector<float> matrix;
};
struct BrainInferenceModel {
  unsigned key = 0, dimension = 0;
  std::vector<BrainInferenceLayer> layers;
};
void validateBrainInference(const BrainInferenceModel &);
std::vector<float> inferOwnerTask(const BrainInferenceModel &,
                                  const std::vector<std::int32_t> &features);
// 414ff0 followed by 40edb0, with a present commander and explicit task
// percent.
float ownerTaskValue(const std::array<float, 3> &probabilities,
                     const std::array<float, 3> &weights, int percent = 100);
} // namespace mnm::reconstruction::combat
