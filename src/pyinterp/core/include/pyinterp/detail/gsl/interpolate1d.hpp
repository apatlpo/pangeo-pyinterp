// Copyright (c) 2019 CNES
//
// All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.
#pragma once
#include "pyinterp/detail/gsl/accelerator.hpp"
#include "pyinterp/detail/broadcast.hpp"
#include <Eigen/Core>
#include <gsl/gsl_interp.h>
#include <memory>

namespace pyinterp {
namespace detail {
namespace gsl {

/// Interpolate a 1-D function
class Interpolate1D {
 public:
  /// Interpolate a 1-D function
  ///
  /// @param type fitting model
  /// @param xa 1-D array of real values.
  /// @param ya 1-D array of real values. The length of ya along the
  /// interpolation axis must be equal to the length of xa.
  /// @param acc
  Interpolate1D(const gsl_interp_type* type,
                const Eigen::Ref<const Eigen::VectorXd>& xa,
                const Eigen::Ref<const Eigen::VectorXd>& ya,
                Accelerator acc = Accelerator())
      : xa_(xa),
        ya_(ya),
        workspace_(std::shared_ptr<gsl_interp>(
            gsl_interp_alloc(type, xa.size()),
            [](gsl_interp* ptr) { gsl_interp_free(ptr); })),
        acc_(std::move(acc)) {
    check_container_size("xa", xa_, "ya", ya_);
    gsl_interp_init(static_cast<gsl_interp*>(*this), xa_.data(), ya_.data(),
                    xa_.size());
  }

  /// Returns the name of the interpolation type used
  inline std::string name() const noexcept {
    return gsl_interp_name(static_cast<const gsl_interp*>(*this));
  }

  /// Return the minimum number of points required by the interpolation
  inline size_t min_size() const noexcept {
    return gsl_interp_min_size(static_cast<const gsl_interp*>(*this));
  }

  /// Return the interpolated value of y for a given point x
  inline double interpolate(const double x) const {
    return gsl_interp_eval(static_cast<const gsl_interp*>(*this), xa_.data(),
                           ya_.data(), x, acc_);
  }

  /// Return the derivative d of an interpolated function for a given point x
  inline double derivative(const double x) const {
    return gsl_interp_eval_deriv(static_cast<const gsl_interp*>(*this),
                                 xa_.data(), ya_.data(), x, acc_);
  }

  /// Return the second derivative d of an interpolated function for a given
  /// point x
  inline double second_derivative(const double x) const {
    return gsl_interp_eval_deriv2(static_cast<const gsl_interp*>(*this),
                                  xa_.data(), ya_.data(), x, acc_);
  }

  /// Return the numerical integral result of an interpolated function over the
  /// range [a, b],
  inline double integral(const double a, const double b) const {
    return gsl_interp_eval_integ(static_cast<const gsl_interp*>(*this),
                                 xa_.data(), ya_.data(), a, b, acc_);
  }

 private:
  const Eigen::Ref<const Eigen::VectorXd> xa_;
  const Eigen::Ref<const Eigen::VectorXd> ya_;
  std::shared_ptr<gsl_interp> workspace_;
  Accelerator acc_;

  /// Gets the GSL pointer
  inline explicit operator gsl_interp*() noexcept { return workspace_.get(); }

  /// Gets the GSL const pointer
  inline explicit operator const gsl_interp*() const noexcept {
    return workspace_.get();
  }
};

}  // namespace gsl
}  // namespace detail
}  // namespace pyinterp
