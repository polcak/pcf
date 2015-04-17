/**
 * Copyright (C) 2009-2012 Libor Polčák <ipolcak@fit.vutbr.cz>
 *                         Barbora Frankova <xfrank08@stud.fit.vutbr.cz>
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

#ifndef _PACKET_TIME_INFO_H
#define _PACKET_TIME_INFO_H

#include <list>
#include <stdint.h>

#include "Point.h"


/**
 * Time information about each packet
 */
class PacketTimeInfo {
  public:
    double ArrivalTime;
    uint64_t Timestamp;
    Point Offset;
};

typedef std::list<PacketTimeInfo> packetTimeInfoList;
typedef packetTimeInfoList::iterator packet_iterator;

#endif
