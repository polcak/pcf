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

#include "clock_skew.h"
#include "observer.h"

/**
 * This class stores information about current known clock skew values
 */
class clock_skew_guard: public observable<const computer_skew>
{
  // Attributes
  private:
    /**
     * Dictionary of known skew of tracked computers
     * IP addresses are mapped to clock skew
     */
    std::map<const std::string, clock_skew_pair> known_skew;

    /**
     * Number of PPM that controls if more addresses are treated as if they
     * belong to the same computer.
     */
    const double THRESHOLD;

    /// Filename of DB with saved computers
    const char *saved_computers;

  // Constructors, destructors
  public:
    /**
     * Constructor
     *
     * @param[in] threshold Number of PPM that means that the address belongs to the same computer
     */
    clock_skew_guard(const double threshold, const char* saved_computers_db): THRESHOLD(threshold), saved_computers(saved_computers_db) {}

    ~clock_skew_guard() {}

  // Private methods
  private:
    void construct_notify(const std::string &ip, const identity_container &identitites, const clock_skew_pair &skew) const;

  // Public methods
  public:
    /**
     * Adds or updates clock skew value of a address
     * @param[in] ip The IP address for which the clock skew is provided
     * @param[in] skew Clock skew of the IP address
     */
    void update_skew(const std::string &ip, const clock_skew_pair &skew);

    /**
     * Returns IP addresses with similar clock skew to the IP address provided
     * @param[in] ip The IP whose clock skew will be compared
     */
    const identity_container get_similar_identities(const std::string &ip);

    /// Returns THRESHOLD
    double get_threshold() {
      return THRESHOLD;
    }
};

#endif
