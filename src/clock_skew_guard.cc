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

#include "clock_skew.h"
#include "clock_skew_guard.h"
#include "check_computers.h"

void clock_skew_guard::construct_notify(const std::string &ip, const identity_container &identitites, const skew &s) const
{
  computer_skew cs = {ip, identitites, s};
  notify(cs);
}


void clock_skew_guard::update_skew(const std::string &ip, const skew &s)
{
  identity_container old_identities = get_similar_identities(ip);

  // Update database, be it a new address or an update
  known_skew[ip] = s;

  // Notify observers
  identity_container new_identities = get_similar_identities(ip);
  construct_notify(ip, new_identities, s);

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

  const skew &ref_skew = ref_it->second;

  if (ref_skew.is_constant()) {
    find_computer_in_saved(ref_skew.get_last_alpha(), identities, THRESHOLD, saved_computers);
  }

  for (auto it = known_skew.begin(); it != known_skew.end(); ++it) {
    if (it->first == ip) {
      // Skip the same address
      continue;
    }

    if (ref_skew.is_similar_with(it->second, THRESHOLD)) {
      identities.insert(it->first);
    }
  }

  return identities;
}
