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

void clock_skew_guard::construct_notify(const std::string &ip, const identity_container &identitites, const clock_skew_atom &skew) const
{
  computer_skew s = {ip, identitites, skew};
  notify(s);
}


void clock_skew_guard::update_skew(const std::string &ip, const clock_skew_atom &skew)
{
  identity_container old_identities = get_similar_identities(ip);

  // Update database
  auto current = known_skew.find(ip);

  if (current == known_skew.end()) {
    // New address
    known_skew[ip] = skew;
    current = known_skew.find(ip);
  }
  else {
    // We are updating the value
    const clock_skew_atom &old_skew = current->second;
    if (old_skew != skew) {
      // Updating skew
      known_skew[ip] = skew;
    }
  }

  // Notify observers
  identity_container new_identities = get_similar_identities(ip);
  construct_notify(ip, new_identities, skew);

  for (auto it = old_identities.begin(); it != old_identities.end(); ++it) {
    if (new_identities.find(*it) == new_identities.end()) {
      construct_notify(*it, get_similar_identities(*it), known_skew[*it]);
    }
  }

  for (auto it = new_identities.begin(); it != new_identities.end(); ++it) {
    if (old_identities.find(*it) == old_identities.end()) {
      construct_notify(*it, get_similar_identities(*it), known_skew[*it]);
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

  double ref_skew = ref_it->second.alpha;

  find_computer_in_saved(ref_skew, identities, THRESHOLD, saved_computers);

  for (auto it = known_skew.begin(); it != known_skew.end(); ++it) {
    if (it->first == ip) {
      // Skip the same address
      continue;
    }

    if (std::fabs(it->second.alpha - ref_skew) <= THRESHOLD) {
      identities.insert(it->first);
    }
  }

  return identities;
}
