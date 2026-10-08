#ifndef frames_hpp__
#define frames_hpp__

#include "Time/Time.hpp"
#include <Eigen/Geometry>

enum class FRAME { BCRF, GCRF, ITRF };

struct Point {
  Eigen::Vector3d r;
};

struct Body : Point {
  Eigen::Vector3d v;
  Eigen::Quaterniond q;
  Eigen::Vector3d w;
};

#endif
