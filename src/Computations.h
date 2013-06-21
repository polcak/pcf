/**
 * Copyright (C) 2012 Jakub Jirasek <xjiras02@stud.fit.vutbr.cz>
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

#ifndef _COMPUTATIONS_H
#define _COMPUTATIONS_H

#include "PacketTimeInfo.h"

class Computations {
public:
/**
 * Swap two points
 * @param[in] x, @param[in] y Points
 */
static void SwapPoints(Point *x, Point *y);

/**
 * Counter-clockwise test
 * 
 * Source: http://en.wikipedia.org/wiki/Graham_scan
 * 
 * @param[in] p1, @param[in] p2, @param[in] p3 Points
 * @return > 0 if counter-clockwise, < 0 if clockwise, = 0 collinear
 */
static double CounterClockwiseTest(Point p1, Point p2, Point p3);

/**
 * Compute and set offsets (x, y)
 * @param[inout] packet The packet whose offset is computed
 * @param[in] head First packet
 * @param[in] freq Frequency
 */
static void SetOffset(PacketTimeInfo &packet, const PacketTimeInfo &head, int freq);

/**
 * Compute upper bound as a upper convex hull. Graham scan algorithm is used.
 * Note that the input list with points is destroyed during the process and
 * the function returns a pointer to the input list.
 * @param[in] points Points list
 * @param[in,out] number List size
 * @return Pointer to list with convex hull points
 */
static Point * ConvexHull(Point points[], unsigned long *number);
};
#endif
