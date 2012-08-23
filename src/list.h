/**
 * Copyright (C) 2012 Jakub Jirasek <xjiras02@stud.fit.vutbr.cz>
 *                    Libor Polčák <ipolcak@fit.vutbr.cz>
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
#include <stdint.h>

// IPv6 addr length (39B) + '\0' + some padding
#define ADDRESS_SIZE 64


/**
 * Stores packets time information. Each packet also contain pointer to next and previous packet
 */
typedef struct packet_time_info packet_time_info;

struct packet_time_info {
  double time;
  uint32_t timestamp;
  struct set {
    // Measured time
    double x;
    // Offset
    double y;
  } offset;
  struct packet_time_info *next_packet;
  struct packet_time_info *prev_packet;
};

/**
 * List of all informations known of all computers. Each computers have pointers to time information
 * about received packets.
 */
typedef struct computer_info computer_info;

struct computer_info {
  packet_time_info *head_packet;
  packet_time_info *tail_packet;
  char address[ADDRESS_SIZE];
  int freq;
  unsigned long count;
  time_t rawtime;
  
  struct {
    double alpha;
    double beta;
  } skew;
  struct computer_info *next_computer;
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
int new_packet(const char *address, double time, uint32_t timestamp);

/**
 * Counts packets in a given list of packets
 * @param[in] head_packet Pointer to the packets list
 * @return Packets count
 */
unsigned long packets_count(packet_time_info *head_packet);

/**
 * Process and print results
 * @param[in] save 1 if save new computer
 * @param[in] uptime 1 if print uptime
 * @param[in] graph 1 if generate graph
 */
void process_results(short save, short uptime, short graph);

/**
 * Free memory used for storing informations about currently known active computers.
 */
void free_memory();


#endif
