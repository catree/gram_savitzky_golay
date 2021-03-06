//  Copyright 2017-2018 CNRS-UM LIRMM
//  Copyright 2017-2018 Arnaud TANGUY <arnaud.tanguy@lirmm.fr>
//
//  This file is part of robcalib.
//
//  robcalib is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  robcalib is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with robcalib.  If not, see <http://www.gnu.org/licenses/>.

#pragma once
#include <gram_savitzky_golay/gram_savitzky_golay.h>
#include <boost/circular_buffer.hpp>
#include <Eigen/Core>
#include <Eigen/Geometry>

namespace gram_sg
{
using Vector6d = Eigen::Matrix< double, 6, 1>;

template <typename T>
class EigenVectorFilter
{
 protected:
  /** Filtering **/
  gram_sg::SavitzkyGolayFilterConfig sg_conf;
  gram_sg::SavitzkyGolayFilter sg_filter;
  // Buffers for Savitzky_golay
  boost::circular_buffer<T> buffer;

 public:
  EigenVectorFilter(const gram_sg::SavitzkyGolayFilterConfig& conf)
      : sg_conf(conf), sg_filter(conf), buffer(2 * sg_filter.config().m + 1)
  {
    reset(T::Zero());
  }

  void reset(const T& data)
  {
    buffer.clear();
    // Initialize to data
    for (size_t i = 0; i < buffer.capacity(); i++)
    {
      buffer.push_back(data);
    }
  }

  void reset()
  {
    reset(T::Zero());
  }

  void clear()
  {
    buffer.clear();
  }

  void add(const T& data) { buffer.push_back(data); }
  T filter() const { return sg_filter.filter(buffer, T::Zero()); }
  gram_sg::SavitzkyGolayFilterConfig config() const
  {
    return sg_conf;
  }

  bool ready() const
  {
    return buffer.size() == buffer.capacity();
  }
};

/**
 * Rotation Filter
 * Based on Peter Cork lecture here:
 * https://www.cvl.isy.liu.se/education/graduate/geometry2010/lectures/Lecture7b.pdf
 * Adapted to real time filtering through Savitzky-Golay
 **/
class RotationFilter
{
  /** Filtering **/
  gram_sg::SavitzkyGolayFilterConfig sg_conf;
  gram_sg::SavitzkyGolayFilter sg_filter;
  // Buffers for Savitzky_golay
  boost::circular_buffer<Eigen::Matrix3d> buffer;

 public:
  RotationFilter(const gram_sg::SavitzkyGolayFilterConfig& conf);
  void reset(const Eigen::Matrix3d& r);
  void reset();
  void clear();
  void add(const Eigen::Matrix3d& r);
  Eigen::Matrix3d filter() const;
  bool ready() const
  {
    return buffer.size() == buffer.capacity();
  }
};

/**
 * @brief Filters Affine3d
 * The transformations are first converted to their translation and RPY
 * compenents, and then each component is filtered individually
 * Finally the result is converted back to an Affine3d
 */
class TransformFilter
{
 private:
  EigenVectorFilter<Eigen::Vector3d> trans_filter;
  RotationFilter rot_filter;

 public:
  TransformFilter(const gram_sg::SavitzkyGolayFilterConfig& conf);
  void reset(const Eigen::Affine3d& T);
  void reset();
  void clear();
  void add(const Eigen::Affine3d& T);
  Eigen::Affine3d filter() const;
  gram_sg::SavitzkyGolayFilterConfig config() const
  {
    return trans_filter.config();
  }
  bool ready() const
  {
    return trans_filter.ready() && rot_filter.ready();
  }
};

class VelocityFilter
{
 private:
  EigenVectorFilter<Vector6d> vfilter;

  Vector6d convert(const Vector6d& T);

 public:
  VelocityFilter(const gram_sg::SavitzkyGolayFilterConfig& conf);
  void reset(const Vector6d& T);
  void reset();
  void clear();
  void add(const Vector6d& T);
  Vector6d filter() const;
  gram_sg::SavitzkyGolayFilterConfig config() const
  {
    return vfilter.config();
  }
  bool ready() const
  {
    return vfilter.ready();
  }
};

}
