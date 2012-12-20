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

#include "computer_info_list.h"

/**
 * Save active computers into file
 * @param[in] all_computers List of active computers
 * @param[in] Active Fileneame of the database with active computers
 * @return 0 if ok
 */
int save_active(const std::list<computer_info> &all_computers, const char *active);

#endif
