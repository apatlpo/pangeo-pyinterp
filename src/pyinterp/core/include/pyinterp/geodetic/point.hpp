// Copyright (c) 2019 CNES
//
// All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.
#pragma once
#include "pyinterp/detail/geometry/point.hpp"
#include <pybind11/pybind11.h>

namespace pyinterp {
namespace geodetic {

// Handle a point in a equatorial spherical coordinates system in degrees.
template <typename T>
class Point2D : public detail::geometry::EquatorialPoint2D<T> {
 public:
  /// Default constructor
  Point2D() : detail::geometry::EquatorialPoint2D<T>() {}

  /// Build a new point with the coordinates provided.
  Point2D(const T& lon, const T& lat)
      : detail::geometry::EquatorialPoint2D<T>(lon, lat) {}

  /// Get longitude value in degrees
  inline T const& lon() const { return this->template get<0>(); }

  /// Get latitude value in degrees
  inline T const& lat() const { return this->template get<1>(); }

  /// Set longitude value in degrees
  inline void lon(T const& v) { this->template set<0>(v); }

  /// Set latitude value in degrees
  inline void lat(T const& v) { this->template set<1>(v); }

  /// Get a tuple that fully encodes the state of this instance
  pybind11::tuple getstate() const {
    return pybind11::make_tuple(lon(), lat());
  }

  /// Create a new instance from a registered state of an instance of this
  /// object.
  static Point2D<T> setstate(const pybind11::tuple& state) {
    if (state.size() != 2) {
      throw std::runtime_error("invalid state");
    }
    return Point2D<T>(state[0].cast<T>(), state[1].cast<T>());
  }

  /// Converts a Point2D into a string with the same meaning as that of this
  /// instance.
  std::string to_string() const {
    std::stringstream ss;
    ss << boost::geometry::dsv(*this);
    return ss.str();
  }
};

}  // namespace geodetic
}  // namespace pyinterp

// BOOST specialization to accept pyinterp::geodectic::Point2D as a geometry
// entity
namespace boost {
namespace geometry {
namespace traits {

namespace pg = pyinterp::geodetic;

/// Coordinate tag
template <typename T>
struct tag<pg::Point2D<T> > {
  /// Typedef for type
  using type = point_tag;
};

/// Coordinate type
template <typename T>
struct coordinate_type<pg::Point2D<T> > {
  /// Typedef for type
  using type = T;
};

/// Coordinate system
template <typename T>
struct coordinate_system<pg::Point2D<T> > {
  /// Typedef for type
  using type = cs::spherical_equatorial<degree>;
};

template <typename T>
struct dimension<pg::Point2D<T> > : boost::mpl::int_<2> {};

/// access struct defining with Cartesian
template <typename T, std::size_t I>
struct access<pg::Point2D<T>, I> {
  /// Accessor to pointer.
  static double get(pg::Point2D<T> const& p) { return p.template get<I>(); }

  /// Pointer setter.
  static void set(pg::Point2D<T>& p, double const& v) {  // NOLINT
    p.template set<I>(v);
  }
};

}  // namespace traits
}  // namespace geometry
}  // namespace boost
