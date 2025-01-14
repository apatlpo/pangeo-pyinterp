// Copyright (c) 2019 CNES
//
// All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.
#pragma once
#include "pyinterp/detail/broadcast.hpp"
#include "pyinterp/detail/math/bicubic.hpp"
#include "pyinterp/detail/thread.hpp"
#include "pyinterp/grid.hpp"

namespace pyinterp {

/// Fitting model
enum FittingModel {
  kLinear,           //!< Linear interpolation
  kPolynomial,       //!< Polynomial interpolation
  kCSpline,          //!< Cubic spline with natural boundary conditions.
  kCSplinePeriodic,  //!< Cubic spline with periodic boundary conditions.
  kAkima,            //!< Non-rounded Akima spline with natural boundary
                     //!< conditions
  kAkimaPeriodic,    //!< Non-rounded Akima spline with periodic boundary
                     //!< conditions
  kSteffen           //!< Steffen’s method guarantees the monotonicity of
                     //!< the interpolating function between the given
                     //!< data points.
};

/// Extension of cubic interpolation for interpolating data points on a
/// two-dimensional regular grid. The interpolated surface is smoother than
/// corresponding surfaces obtained by bilinear interpolation or
/// nearest-neighbor interpolation.
///
/// @tparam Type The type of data used by the numerical grid.
template <typename Type>
class Bicubic : public Grid2D<Type> {
 public:
  /// Default constructor
  using Grid2D<Type>::Grid2D;

  /// Pickle support: set state
  static Bicubic setstate(const pybind11::tuple& tuple) {
    return Bicubic(Grid2D<Type>::setstate(tuple));
  }

  /// Evaluate the interpolation.
  pybind11::array_t<double> evaluate(const pybind11::array_t<double>& x,
                                     const pybind11::array_t<double>& y,
                                     size_t nx, size_t ny,
                                     FittingModel fitting_model,
                                     Axis::Boundary boundary, bool bounds_error,
                                     size_t num_threads) const;

 private:
  /// Loads the interpolation frame into memory
  bool load_frame(double x, double y, Axis::Boundary boundary,
                  bool bounds_error, detail::math::XArray& frame) const;

  /// Returns the GSL interp type
  static const gsl_interp_type* interp_type(const FittingModel kind) {
    switch (kind) {
      case kLinear:
        return gsl_interp_linear;
      case kPolynomial:
        return gsl_interp_polynomial;
      case kCSpline:
        return gsl_interp_cspline;
      case kCSplinePeriodic:
        return gsl_interp_cspline_periodic;
      case kAkima:
        return gsl_interp_akima;
      case kAkimaPeriodic:
        return gsl_interp_akima_periodic;
      case kSteffen:
        return gsl_interp_steffen;
      default:
        throw std::invalid_argument("Invalid interpolation type: " +
                                    std::to_string(kind));
    }
  }

  /// Pickle support: derived class construction from the base class.
  explicit Bicubic(Grid2D<Type>&& grid) : Grid2D<Type>(grid) {}
};
}  // namespace pyinterp
