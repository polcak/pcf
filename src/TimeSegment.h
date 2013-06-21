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

#ifndef _TIME_SEGMENT_H
#define _TIME_SEGMENT_H

#include <limits>
#include <set>
#include <string>

/**
 * Clock skew y = alpha*x + beta, valid in certain time
 */
class TimeSegment {
public:
  double alpha;
  double beta;
  /// Validity
  double startTime;
  double endTime;
  double relativeStartTime;
  double relativeEndTime;

  bool operator==(const TimeSegment &other) const
  {
    return alpha == other.alpha &&
      beta == other.beta &&
      startTime == other.startTime &&
      endTime == other.endTime;
  }

  bool operator!=(const TimeSegment &other) const
  {
    return !(*this == other);
  }
};


/** TODO: move to global helper
 * 
 * Containter for identitites of a computer.
 *
 * It is usually an IP address or an arbitrary name of a computer.
 */
typedef std::set<std::string> identity_container;


/** TODO: move to global helper
 * 
 * Undefined skew
 */
const double UNDEFINED_SKEW = std::numeric_limits<double>().quiet_NaN();
#endif
