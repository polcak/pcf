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

#include <algorithm>

#include "skew.h"

bool skew::is_similar_with(const skew &other, const double THRESHOLD) const
{
  if (is_constant() && other.is_constant()) {
    return similar_alpha(atoms.begin()->alpha, other.atoms.begin()->alpha, THRESHOLD);
  }

  return compare_changing(other, THRESHOLD);
}



bool skew::compare_changing(const skew &other, const double THRESHOLD) const
{
  double both_active = 0.0;
  double similar_skew = 0.0;

  auto itt = atoms.begin();
  auto ito = other.atoms.begin();

  while (itt != atoms.end() && ito != other.atoms.end()) {
    if (itt->end_time < ito->start_time) {
      ++itt;
    }
    else if (ito->end_time < itt->start_time) {
      ++ito;
    }
    else {
      double start_time = std::max(itt->start_time, ito->start_time);
      double end_time = start_time;
      bool sa = similar_alpha(itt->alpha, ito->alpha, THRESHOLD);

      if (itt->end_time < ito->end_time) {
        end_time = itt->end_time;
        ++itt;
      }
      else {
        end_time = ito->end_time;
        ++ito;
      }

      double overlap_time = end_time - start_time;
      both_active += overlap_time;
      if (sa) {
        similar_skew += overlap_time;
      }
    }
  }

  return (both_active > 0.0 && (similar_skew > SIMILAR_SKEW_PERCENTAGE * both_active));
}
