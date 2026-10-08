#ifndef frames_frames_hpp__
#define frames_frames_hpp__

#include "Time/Time.hpp"
#include <Eigen/Geometry>

enum class FRAME { BCRF, GCRF, ITRF };

template <FRAME F> struct Point {
  Eigen::Vector3d r;
};

template <FRAME F> struct PointBody : Point<F> {
  Eigen::Vector3d v;
};

template <FRAME F> struct Rotation {
  Eigen::Quaterniond q;
  Eigen::Vector3d w;
};

template <FRAME F> struct Body : PointBody<F>, Rotation<F> {};

#endif
