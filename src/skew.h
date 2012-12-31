/**
 * Copyright (C) 2012 Libor Polčák <ipolcak@fit.vutbr.cz>
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

#include "clock_skew.h"

class skew
{
  // Attributes
  private:
    /// List of atomic skews
    std::list<clock_skew_atom> atoms;
    /**
     * Percentage of similar skew during overlaping periods of two skews so
     * they are considered to be similar. The percentage is actually divided
     * by 100 so the number is between 0.0 and 1.0.
     */
    static const double SIMILAR_SKEW_PERCENTAGE = 0.9;
    /**
     * End time of the meassurement
     */
    double end_time;

  // Constructors, destructors
  public:
    skew(): atoms(), end_time(0.0) {}

  // Public methods
  public:
    /// Adds new atom
    void add_atom(const clock_skew_atom &atom)
    {
      atoms.push_back(atom);
      set_end_time(atom.end_time);
    }

    /**
     * Allows manually set end time. But it has to be larger than the
     * previous value.
     */
    double set_end_time(double new_end)
    {
      end_time = std::max(end_time, new_end);
      return end_time;
    }

    /// Returns if the clock skew is constant or if it is changing over time
    bool is_constant() const
    {
      return atoms.size() == 1;
    }

    /// Returns last known alpha
    double get_last_alpha() const
    {
      return atoms.rbegin()->alpha;
    }

    double get_start_time() const
    {
      return atoms.cbegin()->start_time;
    }

    double get_end_time() const
    {
      return end_time;
    }

    /// Compares if the other skew is similar to this one
    bool is_similar_with(const skew &other, const double THRESHOLD) const;

    /// Iterators to stored atoms
    std::list<clock_skew_atom>::const_iterator cbegin() const
    {
      return atoms.cbegin();
    }

    std::list<clock_skew_atom>::const_iterator cend() const
    {
      return atoms.cend();
    }

  // Private methods
  private:
    static bool similar_alpha(double alpha1, double alpha2, const double THRESHOLD)
    {
      return (std::fabs(alpha1 - alpha2) < THRESHOLD);
    }

    bool compare_changing(const skew &other, const double THRESHOLD) const;
};

#endif
