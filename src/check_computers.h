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

#ifndef _CHECK_COMPUTERS_H
#define _CHECK_COMPUTERS_H

#include "TimeSegment.h"
#include "ComputerInfoList.h"

/**
 * Conversts time to its string representation in human readable format
 * @param[out] buffer       Pre-allocated buffer where the output is stored
 * @param[in] buffer_size   Size of the buffer
 * @param[in] time          Unix time to be converted
 */
void time_to_str(char *buffer, size_t buffer_size, time_t time);

/**
 * Search for computers with similar skew in saved computers
 * @param[in] referenced_skew Skew to be searched
 * @param[inout] identities Container with identities of the computer that is being searched for
 * @param[in] THRESHOLD threshold for the similar skew
 * @param[in] database Filename of XML database with saved computers and their skew
 * @return true if success, false otherwise
 */
bool find_computer_in_saved(double referenced_skew, identity_container &identities, const double THRESHOLD, const char *database);

/**
 * Save active computers into file
 * @param[in] all_computers List of active computers
 * @param[in] Active Fileneame of the database with active computers
 * @return 0 if ok
 */
int save_active(const std::list<ComputerInfo *> &all_computers, const char *active, ComputerInfoList &computers);

#endif
