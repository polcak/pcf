/**
 * Copyright (C) 2012 Jakub Jirasek <xjiras02@stud.fit.vutbr.cz>
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

#include "computer_info.h"
#include "packet_time_info.h"


/**
 * Compute and set offsets (x, y)
 * @param[inout] packet The packet whose offset is computed
 * @param[in] head First packet
 * @param[in] freq Frequency
 */
void set_offset(packet_time_info &packet, const packet_time_info &head, int freq);

/**
 * Compute upper bound as a upper convex hull. Graham scan algorithm is used.
 * Note that the input list with points is destroyed during the process and
 * the function returns a pointer to the input list.
 * @param[in] points Points list
 * @param[in,out] number List size
 * @return Pointer to list with convex hull points
 */
point2d *convex_hull(point2d points[], unsigned long *number);

#endif
