/**
 * Copyright (C) 2013 Barbora Frankova <xfrank08@stud.fit.vutbr.cz>
 *                    Libor Polcak <ipolcak@fit.vutbr.cz>
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

#ifndef PACKETSEGMENT_H
#define	PACKETSEGMENT_H
#include "PacketTimeInfo.h"

class PacketSegment {
    /// Structure describing clock skew
public:
      double alpha;
      double beta;
      double confirmedAlpha;
      double confirmedBeta;
      packet_iterator first;
      packet_iterator confirmed;
      packet_iterator last;
};


#endif
