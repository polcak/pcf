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

#ifndef _GNUPLOT_GRAPH_H
#define _GNUPLOT_GRAPH_H

#include "clock_skew.h"
#include "observer.h"

class gnuplot_graph: public observer<const computer_skew>
{
  public:
    virtual void notify(const computer_skew& changed_skew);

  private:
    void generate_graph(const computer_skew& changed_skew);
};

#endif
