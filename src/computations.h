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

#include "list.h"


/**
 * Computes clock frequency according to given packet set
 * @param[in] packets Pointer to the list of packets
 * @return Frequency
 */
int compute_clock_frequency(packet_time_info *packets);

/**
 * Compute and set offsets (x, y)
 * @param[in] head Pointer to the first packet
 * @param[in] from Pointer to the packet from where to set offsets till end
 * @param[in] freq Frequency
 */
void set_offsets(packet_time_info *head, packet_time_info *from, int freq);

/**
 * Compute and set clock skew
 * @param[in] list Pointer to header of packet list
 * @return 0 if ok
 */
int set_skew(computer_info *list);


#endif
