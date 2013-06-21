/**
 * Copyright (C) 2012 Libor Polčák <ipolcak@fit.vutbr.cz>
 *                    Barbora Frankova <xfrank08@stud.fit.vutbr.cz>
 * 
 * This file is part of pcf - PC fingerprinter.
 *
 * Pcf is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Pcf is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with pcf. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _SKEW_H
#define _SKEW_H

#include <cmath>
#include <list>

#include "TimeSegment.h"

class TimeSegmentList
{
  // Attributes
  private:
    /// List of atomic skews
    std::list<TimeSegment> atoms;
    /**
     * Percentage of similar skew during overlaping periods of two skews so
     * they are considered to be similar. The percentage is actually divided
     * by 100 so the number is between 0.0 and 1.0.
     */
    constexpr static double skewCorrespondence = 0.9;
    /**
     * End time of the meassurement
     */
    double endTime;

  // Constructors, destructors
  public:
    TimeSegmentList(): atoms(), endTime(0.0) {}

    /// Adds new atom
    void add_atom(const TimeSegment &atom)
    {
      atoms.push_back(atom);
      set_end_time(atom.endTime);
    }

    /**
     * Allows manually set end time. But it has to be larger than the
     * previous value.
     */
    double set_end_time(double new_end) {
      endTime = std::max(endTime, new_end);
      return endTime;
    }

    /// Returns if the clock skew is constant or if it is changing over time
    bool is_constant() const {
      return atoms.size() == 1;
    }

    /// Returns last known alpha
    double get_last_alpha() const {
      return atoms.rbegin()->alpha;
    }

    double get_start_time() const {
      return atoms.cbegin()->startTime;
    }

    double get_end_time() const {
      return endTime;
    }

    /// Compares if the other skew is similar to this one
    bool is_similar_with(const TimeSegmentList &other, const double THRESHOLD) const;

    /// Iterators to stored atoms
    std::list<TimeSegment>::const_iterator cbegin() const
    {
      return atoms.cbegin();
    }

    std::list<TimeSegment>::const_iterator cend() const
    {
      return atoms.cend();
    }

  // Private methods
  private:
    static bool similar_alpha(double alpha1, double alpha2, const double THRESHOLD){
      return (std::fabs(alpha1 - alpha2) < THRESHOLD);
    }

    bool compare_changing(const TimeSegmentList &other, const double THRESHOLD) const;
};

#endif
