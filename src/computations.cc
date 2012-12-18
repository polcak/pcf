/**
 * Copyright (C) 2012 Jakub Jirasek <xjiras02@stud.fit.vutbr.cz>
 *                    Libor Polčák <ipolcak@fit.vutbr.cz>
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

#include "point2d.h"
#include "packet_time_info.h"


/**
 * Swap two points
 * @param[in] x, @param[in] y Points
 */
void swap(point2d *x, point2d *y);

/**
 * Counter-clockwise test
 * 
 * Source: http://en.wikipedia.org/wiki/Graham_scan
 * 
 * @param[in] p1, @param[in] p2, @param[in] p3 Points
 * @return > 0 if counter-clockwise, < 0 if clockwise, = 0 collinear
 */
double ccw(point2d p1, point2d p2, point2d p3);


void swap(point2d *x, point2d *y)
{
   point2d tmp;
   tmp = *x;
   *x = *y;
   *y = tmp;
}

double ccw(point2d p1, point2d p2, point2d p3)
{
  return((p2.x - p1.x) * (p3.y - p1.y) - (p2.y - p1.y) * (p3.x - p1.x));
}

// More details in:
// Graham, R. L.: An efficient algorithm for determining the convex hull of a finite
// planar set. Information Processing Letters, vol. 1, no. 4, jan 1972: pp. 132–133,
// ISSN 0020-0190.
point2d *convex_hull(point2d points[], unsigned long *number)
{
  unsigned long i;
  int m = 1;
  
  for (i = 2; i < *number; i++) {
    while (ccw(points[m - 1], points[m], points[i]) >= 0) {
      if (m == 1) {
        swap(&points[m], &points[i]);
        i++;
      }
      else
        m--;
    }
    m++;
    swap(&points[m], &points[i]);
  }
  
  *number = ++m;
  
  return(points);
}


void set_offset(packet_time_info &packet, const packet_time_info &head, int freq)
{
  double tmp;

  packet.offset.x = packet.time - head.time;

  tmp = packet.timestamp - head.timestamp;
  tmp /= freq;
  tmp -= packet.offset.x;

  // s -> ms
  tmp *= 1000;

  packet.offset.y = tmp;
}

