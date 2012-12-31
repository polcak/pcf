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

#include <cmath>

#include "computer_info_list.h"
#include "check_computers.h"


computer_info_list::~computer_info_list()
{
  for (auto it = computers.begin(); it != computers.end(); ++it) {
    delete *it;
  }
}



void computer_info_list::new_packet(const char *address, double ttime, uint32_t timestamp)
{
  static unsigned long total = 0;
  printf("\r%lu packets captured", ++total);
  fflush(stdout);

  bool found = false;
  for (auto it = computers.begin(); it != computers.end(); ++it) {
    if ((*it)->get_address() != address) {
      continue;
    }
    found = true;

    // Computer already known
    computer_info &known_computer = **it;

    /// Too much time since last packet so start from the beginning
    if ((ttime - known_computer.get_last_packet_time()) > TIME_LIMIT) {
      known_computer.restart(ttime, timestamp);
#ifdef DEBUG
      fprintf(stderr, "%s timeout: starting a new tracking\n", known_computer.get_address().c_str());
#endif
      break;
    }

    // Check if packet has the same or lower timestamp
    if (timestamp <= known_computer.get_last_packet_timestamp()) {
#ifdef DEBUG
      if (timestamp < known_computer.get_last_packet_timestamp())
        fprintf(stderr, "%s: Lower timestamp %u %u\n", known_computer.get_address().c_str(), timestamp, known_computer.get_last_packet_timestamp());
#endif
      break;
    }

    // Stop supporting lists with stupid frequency
    if (std::fabs(known_computer.get_freq()) > 10000) {
#ifdef DEBUG
      fprintf(stderr, "%s: too high frequency of %d\n", known_computer.get_address().c_str(), known_computer.get_freq());
#endif
      known_computer.restart(ttime, timestamp);
      break;
    }

    // Insert packet
    known_computer.insert_packet(ttime, timestamp, skews);

#if 0
    std::cerr << known_computer.get_address() << ": " << known_computer.get_packet_count() << std::endl;
#endif
  }

  if (!found) {
    computer_info *new_computer = new computer_info(ttime, timestamp, address, block);
    computers.push_back(new_computer);
    save_active(computers, active, skews);
  }

  if (ttime > (last_inactive + TIME_LIMIT / 4)) {
    /// Save active computers
    for (auto it = computers.begin(); it != computers.end(); ++it) {
      if (ttime - (*it)->get_last_packet_time() > TIME_LIMIT) {
        delete(*it);
        it = computers.erase(it);
      }
    }

    save_active(computers, active, skews);
    last_inactive = ttime;
  }
}
