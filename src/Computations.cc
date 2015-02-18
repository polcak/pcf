/**
 * Copyright (C) 2012 Jakub Jirasek <xjiras02@stud.fit.vutbr.cz>
 *                    Libor Polčák <ipolcak@fit.vutbr.cz>
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

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "Point.h"
#include "PacketTimeInfo.h"
#include "Computations.h"
#include "Configurator.h"

void Computations::SwapPoints(Point *x, Point *y) {
   Point tmp;
   tmp = *x;
   *x = *y;
   *y = tmp;
}

double Computations::CounterClockwiseTest(Point p1, Point p2, Point p3) {
  return((p2.x - p1.x) * (p3.y - p1.y) - (p2.y - p1.y) * (p3.x - p1.x));
}

// More details in:
// Graham, R. L.: An efficient algorithm for determining the convex hull of a finite
// planar set. Information Processing Letters, vol. 1, no. 4, jan 1972: pp. 132–133,
// ISSN 0020-0190.
Point * Computations::ConvexHull(Point points[], unsigned long *number)
{
  unsigned long i;
  int m = 1;
  
  for (i = 2; i < *number; i++) {
    while (i <= *number && Computations::CounterClockwiseTest(points[m - 1], points[m], points[i]) >= 0) {
      if (m == 1) {
        Computations::SwapPoints(&points[m], &points[i]);
        i++;
      }
      else
        m--;
    }
    m++;
    Computations::SwapPoints(&points[m], &points[i]);
  }
  
  *number = ++m;
  
  return(points);
}


void Computations::SetOffset(PacketTimeInfo &packet, const PacketTimeInfo &head, int freq){
  double tmp;

  packet.Offset.x = packet.ArrivalTime - head.ArrivalTime;

  tmp = packet.Timestamp - head.Timestamp;
  tmp /= freq;
  tmp -= packet.Offset.x;
  
  tmp *= 1000;

  packet.Offset.y = tmp;
}

