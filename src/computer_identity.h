/**
 * Copyright (C) 2009-2012 Libor Polčák <ipolcak@fit.vutbr.cz>
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

// FIXME rewrite to C++
#if 0
#ifndef _COMPUTER_IDENTITY_H
#define _COMPUTER_IDENTITY_H

#include "list.h"

/**
 * One item in a list of computers known either by name or by IP address
 */
typedef struct computer_identity_item computer_identity_item;
struct computer_identity_item {
  /// Its kown name or address
  char *name_address;
  /// Its known skew
  double skew_diff;
  /// Next Item
  struct computer_identity_item *next;
};

/**
 * List structure of computers known either by name or by IP address
 */
typedef struct computer_identity_list
{
  /// IP address of the referenced computer
  char referenced_address[ADDRESS_SIZE];
  /// Clock skew of the referenced computer
  double referenced_skew;
  /// Pointer to the first similar computer in the list
  computer_identity_item* first;
} computer_identity_list;

/**
 * Initialization
 * @param[inout] list The list to be initialised
 * @param[in] address IP address of the computer which identity is searched
 */
void computer_identity_list_init(computer_identity_list *list, const char *address, double skew);

/**
 * Releases all memory allocated for the list items
 * @param[inout] list The list to be released
 */
void computer_identity_list_release(computer_identity_list *list);

/**
 * Mallocs space for an item, sets its values and inserts it to the list.
 * List's actual item is set to the just created one.
 */
computer_identity_item *computer_identity_list_add_item(computer_identity_list* list, const char* name_address, double skew);

#endif
#endif
