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

#include "SkewChangeExporter.h"

#include <iostream>

void SkewChangeExporter::Notify(const AnalysisInfo& changed_skew)
{
#ifdef DEBUG
  printf("SkewChangeExporter::notify %s\n", changed_skew.Address.c_str());
#endif
  std::cout << source_type << '\t' << changed_skew.Address << '\t';
  for (auto it = changed_skew.SimilarIdentities.begin(); it != changed_skew.SimilarIdentities.end(); ++it) {
    std::cout << *it << '\t';
  }
  std::cout << std::endl;
}


