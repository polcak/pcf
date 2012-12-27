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

#ifndef _CLOCK_SKEW_H
#define _CLOCK_SKEW_H

#include <limits>
#include <set>
#include <string>
#include <utility>

/**
 * Clock skew pair (alpha, beta): y = alpha*x + beta
 */
typedef std::pair<double, double> clock_skew_pair;


/**
 * Containter for identitites of a computer.
 *
 * It is usually an IP address or an arbitrary name of a computer.
 */
typedef std::set<std::string> identity_container;


/**
 * Undefined skew
 */
const double UNDEFINED_SKEW = std::numeric_limits<double>().quiet_NaN();

/**
 * Structure that is used to transfer computed data about one computer
 * between classes.
 */
struct computer_skew {
  const std::string& address;
  identity_container similar_identities;
  double clock_skew;
};
#endif
