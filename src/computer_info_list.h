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

#ifndef _COMPUTER_INFO_LIST_H
#define _COMPUTER_INFO_LIST_H

#include "computer_info.h"

/**
 * All informations known about a set of computers.
 */
class computer_info_list {
  // Attributes
  private:
    /// Informations about packet timing
    std::list<computer_info> computers;
    /// Last time when inactive computers were detected
    double last_inactive;

    // Program configuration
    // Filenames of databases
    char *active;
    char *database;

    /// Number of packets in one block
    const int block;
    /**
     * Number of seconds of inactivity afeter which old data are erased
     * and the tracking is restarted
     */
    const int time_limit;
    /**
     * Number of PPM that controls if more addresses are treated as if they
     * belong to the same computer.
     */
    const double threshold;

  // Constructors
  public:
    computer_info_list(char *_active, char *_database, int _block, int _time_limit, double _threshold):
      last_inactive(time(NULL)), active(_active), database(_database), block(_block), time_limit(_time_limit), threshold(_threshold)
    {}

  // Destructor
  public:
    ~computer_info_list() {}

  // Public methods
  public:
    /**
     * New packet processing (classify, save, compute...)
     * @param[in] address IP address of the source
     * @param[in] time Real time when packet arrived
     * @param[in] timestamp PCAP timestamp of the packet
     * @return 0 if ok
     */
    void new_packet(const char *address, double time, uint32_t timestamp);
};

#endif
