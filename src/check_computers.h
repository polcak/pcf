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
 * Check new skew in saved computers
 * @param[in] skew Actual measured skew
 * @return Name of the computer if found, NULL otherwise
 */
char *check_computers(double skew, double *diff);

/**
 * Check new skew in active computers
 * @param[in] list List of active computers
 * @param[in] current_list Computer to be checked
 * @return Name of the computer if found, NULL otherwise
 */
char *check_actives(computer_info *list, computer_info *current_list);

/**
 * Save active computers into file
 * @param[in] list List of active computers
 * @return 0 if ok
 */
int save_active(computer_info *list);


#endif
