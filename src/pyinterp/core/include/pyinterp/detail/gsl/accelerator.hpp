// Copyright (c) 2019 CNES
//
// All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.
#pragma once
#include <gsl/gsl_interp.h>
#include <memory>

namespace pyinterp {
namespace detail {
namespace gsl {

/// Kind of iterator for interpolation lookups. It caches the previous value of
/// an index lookup. When the subsequent interpolation point falls in the same
/// interval its index value can be returned immediately.
class Accelerator {
 public:
  /// Default constructor
  Accelerator()
      : acc_(std::shared_ptr<gsl_interp_accel>(
            gsl_interp_accel_alloc(),
            [](gsl_interp_accel* ptr) { gsl_interp_accel_free(ptr); })) {}

  /// Gets the GSL pointer
  inline operator gsl_interp_accel*() const noexcept {  // NOLINT
    return acc_.get();
  }

 private:
  std::shared_ptr<gsl_interp_accel> acc_;
};

}  // namespace gsl
}  // namespace detail
}  // namespace pyinterp
