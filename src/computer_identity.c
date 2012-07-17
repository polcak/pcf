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


#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "computer_identity.h"
#include "list.h"

void computer_identity_list_init(computer_identity_list *list, const char *address, double skew)
{
  strncpy(list->referenced_address, address, ADDRESS_SIZE);
  list->referenced_skew = skew;
  list->first = NULL;
}

void computer_identity_list_release(computer_identity_list *list)
{
  for (computer_identity_item *act = list->first; act != NULL;)
  {
    computer_identity_item *next = act->next;
    free(act->name_address);
    free(act);
    act = next;
  }

  list->first = NULL;
}

computer_identity_item *computer_identity_list_add_item(computer_identity_list* list, const char* name_address, double skew)
{
  /* Malloc required space */
  computer_identity_item *new_item;
  if ((new_item = malloc(sizeof(computer_identity_item))) == NULL)
  {
    return NULL;
  }

  new_item->next = list->first;

  size_t name_address_bytes = strlen(name_address) + 1;
  new_item->name_address = malloc(name_address_bytes);
  if (new_item->name_address == NULL)
  {
    return NULL;
  }
  strncpy(new_item->name_address, name_address, name_address_bytes);
  new_item->skew_diff = fabs(skew - list->referenced_skew);

  list->first = new_item;
  return new_item;
}
