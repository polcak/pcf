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

#ifndef _CLOCK_SKEW_GUARD_H
#define _CLOCK_SKEW_GUARD_H

#include <map>
#include <string>
#include <set>

/**
 * This class stores information about current known clock skew values
 */
class clock_skew_guard
{
  // Attributes
  private:
    /**
     * Dictionary of known skew of tracked computers
     * IP addresses are mapped to clock skew
     */
    std::map<const std::string, double> known_skew;

    /**
     * Number of PPM that controls if more addresses are treated as if they
     * belong to the same computer.
     */
    const double THRESHOLD;

  // Constructors, destructors
  public:
    /**
     * Constructor
     *
     * @param[in] threshold Number of PPM that means that the address belongs to the same computer
     */
    clock_skew_guard(const double threshold): THRESHOLD(threshold) {}

    ~clock_skew_guard() {}

  // Type definitions
  public:
    typedef std::set<std::string> address_containter;

  // Public methods
  public:
    /**
     * Adds or updates clock skew value of a address
     * @param[in] ip The IP address for which the clock skew is provided
     * @param[in] skew Clock skew of the IP address
     */
    void update_skew(const std::string &ip, double skew);

    /**
     * Returns IP addresses with similar clock skew to the IP address provided
     * @param[in] ip The IP whose clock skew will be compared
     */
    const address_containter get_similar_identities(const std::string &ip);
};

#endif
