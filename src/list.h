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

#ifndef _LIST_H
#define _LIST_H

#include <time.h>


/**
 * Packets list structure
 */
typedef struct my_pkt my_packet;

struct my_pkt {
  double time;
  unsigned long int timestamp;
  struct set {
    // Measured time
    double x;
    // Offset
    double y;
  } offset;
  struct my_pkt *next;
  struct my_pkt *prev;
};

/**
 * List of all packet lists (header for every packet list)
 */
typedef struct my_lst my_list;

struct my_lst {
  my_packet *head;
  my_packet *tail;
  char address[16];
  int freq;
  char *name;
  unsigned long count;
  time_t rawtime;
  
  /// Manage saving
  short first;
  
  struct {
    double alpha;
    double beta;
    double diff;
  } skew;
  struct my_lst *next;
};

/// Files
char *active;
char *database;

/// Program constants
int BLOCK;
int TIME_LIMIT;
double THRESHOLD;

/**
 * New packet processing (classify, save, compute...)
 * @param[in] address IP address of the source
 * @param[in] time Real time when packet arrived
 * @param[in] timestamp Timestamp of the packet
 * @return 0 if ok
 */
int new_packet(const char *address, double time, unsigned long int timestamp);

/**
 * Packets count
 * @param[in] head Pointer to the packets list
 * @return Packets count
 */
unsigned long packets_count(my_packet *head);

/**
 * Process and print results
 * @param[in] save 1 if save new computer
 * @param[in] uptime 1 if print uptime
 * @param[in] graph 1 if generate graph
 */
void process_results(short save, short uptime, short graph);

/**
 * Free all memory
 */
void free_memory();


#endif
