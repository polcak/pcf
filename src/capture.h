/**
 * Copyright (C) 2012 Jakub Jirasek <xjiras02@stud.fit.vutbr.cz>
 *                    Barbora Frankova <xfrank08@stud.fit.vutbr.cz>
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

#ifndef _CAPTURE_H
#define _CAPTURE_H

#include <arpa/inet.h>
#include <pcap.h>
#include <string>


    /**
     * Capture packets
     * @param[in] config   Config data
     * @return 0           if ok
     */
     int StartCapturing();

    /**
     * Signal handler to stop capturing
     */
     void StopCapturing(int signum);

    /** 
     * Callback function for pcap
     * @param[in] args     User params
     * @param[in] header   Packet header
     * @param[in] packet   Packet
     */
     void GotPacket(u_char *args, const struct pcap_pkthdr *header, const u_char *packet);
     
     typedef struct {
       u_int16_t pckt_type;
       u_int16_t arp_type;
       u_int16_t link_addr_len;
       u_char link_addr[8];
       u_int16_t proto_type;
     } linux_cooked_hdr ;
#endif
