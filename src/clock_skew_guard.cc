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

#include <cmath>

#include "clock_skew.h"
#include "clock_skew_guard.h"
#include "check_computers.h"

void clock_skew_guard::update_skew(const std::string &ip, double skew)
{
  auto current = known_skew.find(ip);

  if (current == known_skew.end()) {
    // New address
    known_skew[ip] = skew;
  }
  else {
    // We are updating the value
    double old_skew = current->second;
    if (old_skew != skew) {
      // Updating skew
      known_skew[ip] = skew;
    }
  }
}



const identity_container clock_skew_guard::get_similar_identities(const std::string &ip)
{
  identity_container identities;

  auto ref_it = known_skew.find(ip);
  if (ref_it == known_skew.end()) {
    // Given address is not known
    return identities;
  }

  double ref_skew = ref_it->second;

  find_computer_in_saved(ref_skew, identities, THRESHOLD, saved_computers);

  for (auto it = known_skew.begin(); it != known_skew.end(); ++it) {
    if (it->first == ip) {
      // Skip the same address
      continue;
    }

    if (std::fabs(it->second - ref_skew) <= THRESHOLD) {
      identities.insert(it->first);
    }
  }

  return identities;
}
