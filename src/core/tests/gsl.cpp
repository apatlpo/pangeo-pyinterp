#include "pyinterp/detail/gsl/interpolate1d.hpp"
#include "pyinterp/detail/gsl/error_handler.hpp"
#include <gtest/gtest.h>

namespace gsl = pyinterp::detail::gsl;

TEST(gsl, interpolate) {
  // Test copied from a unit test of the GSL library.
  gsl::set_error_handler();

  auto data_x = std::array<double, 3>{0.0, 1.0, 2.0};
  auto data_y = std::array<double, 3>{0.0, 1.0, 2.0};
  auto test_x = std::array<double, 4>{0.0, 0.5, 1.0, 2.0};
  auto test_y = std::array<double, 4>{0.0, 0.5, 1.0, 2.0};
  auto test_dy = std::array<double, 4>{1.0, 1.0, 1.0, 1.0};
  auto test_iy = std::array<double, 4>{0.0, 0.125, 0.5, 2.0};

  auto interpolator = gsl::Interpolate1D<3>(gsl_interp_cspline, data_x, data_y);

  for (auto ix = 0; ix < 4; ix++) {
    auto x = test_x[ix];
    EXPECT_NEAR(interpolator.interpolate(x), test_y[ix], 1e-10);
    EXPECT_NEAR(interpolator.derivative(x), test_dy[ix], 1e-10);
    EXPECT_NEAR(interpolator.integral(test_x[0], x), test_iy[ix], 1e-10);
  }
}

TEST(gsl, exception) {
  gsl::set_error_handler();

  auto data_x = std::array<double, 1>{0.0};
  auto data_y = std::array<double, 1>{0.0};

  EXPECT_THROW(gsl::Interpolate1D<1>(gsl_interp_cspline, data_x, data_y),
               std::runtime_error);
}