#pragma once
#include <torch/expanding_array.h>
#include <torch/nn/cloneable.h>

#include <torch/csrc/WindowsTorchApiMacro.h>

namespace torch {
namespace nn {
namespace utils {

inline float clip_grad_norm_(
    std::vector<Tensor>& parameters,
    float max_norm,
    float norm_type = 2.0) {
  std::vector<Tensor> params_with_grad;

  for (const auto& param : parameters) {
    auto& grad = param.grad();
    if (grad.defined()) {
      params_with_grad.push_back(param);
    }
  }
  float total_norm = 0.0;
  float inf = std::numeric_limits<float>::infinity();
  if (norm_type == inf) {
    for (const auto& param : params_with_grad) {
      auto param_max = param.grad().abs().max().item().toFloat();
      if (param_max > total_norm) {
        total_norm = param_max;
      }
    }
  } else {
    for (const auto& param : params_with_grad) {
      auto param_norm = torch::norm(param.grad(), norm_type);
      total_norm += torch::pow(param_norm, norm_type).item().toFloat();
    }
    total_norm = std::pow(total_norm, 1.0 / norm_type);
  }

  const auto EPS = 1e-6;
  auto clip_coef = max_norm / (total_norm + EPS);
  if (clip_coef < 1) {
    for (auto& param : params_with_grad) {
      param.grad().mul_(clip_coef);
    }
  }
  return total_norm;
}

// A wrapper around clip_grad_norm_ that allows us to call the function with a
// single Tensor.
inline float clip_grad_norm_(
    Tensor& parameters,
    float max_norm,
    float norm_type = 2.0) {
  std::vector<Tensor> params = {parameters};
  return clip_grad_norm_(params, max_norm, norm_type);
}

} // namespace utils
} // namespace nn
} // namespace torch
