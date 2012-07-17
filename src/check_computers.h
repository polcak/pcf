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

#ifndef _CHECK_COMPUTERS_H
#define _CHECK_COMPUTERS_H

#include "list.h"
#include "computer_identity.h"

/**
 * Save new computer
 * Function not used any more (was in interactive mode)
 * @param[in] name New computer name
 * @param[in] skew Skew
 * @param[in] freq Frequency
 * @param[in] address Address
 * @return 0 if ok
 */
int save_computer(const char *name, double skew, int freq, const char *address);

/**
 * Search for computers with similar skew in saved computers
 * @param[in] address IP address of the computer to be searched
 * @param[in] skew Skew of the computer that is beign search for
 * @param[in] known_computers List of computers to be searched
 * @return computer_identity_list filled with computers with similar clock skew
 */
computer_identity_list *find_computers_by_skew(const char *address, double skew, computer_info *known_computers);

/**
 * Save active computers into file
 * @param[in] list List of active computers
 * @return 0 if ok
 */
int save_active(computer_info *list);


#endif
