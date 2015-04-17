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

#ifndef _COMPUTER_INFO_H
#define _COMPUTER_INFO_H

#include <list>
#include <string>
#include <utility>

#include "TimeSegment.h"
#include "PacketTimeInfo.h"
#include "TimeSegmentList.h"
#include "ClockSkewPair.h"
#include "PacketSegment.h"

/**
 * All informations known about each computer including time information about all received packets.
 */
class ComputerInfo {

  // Private types
  private:
    /// List of time informations about packets
    packetTimeInfoList packets;

    /// Frequency of the computer
    int freq;

    /// Time of the last added packet
    double lastPacketTime;

    /// Time of the packet with last confirmed skew
    double lastConfirmedPacketTime;

    /// Last confirmed skew
    ClockSkewPair confirmedSkew;

    /// Time of the start of the capture
    double startTime;

    /// Skew information about one computer
    std::list<PacketSegment> packetSegmentList;
    
    // pointer to the parent list of computers that includes this one
    void * parentList;

    /// Complete address of the computer
    std::string address;
    
    /// IP address of the computer
    const std::string ipAddress;

    ///
    const uint16_t port;
    
    double variance;
    double avg;
    double dev;
    unsigned long long int numOfPackets;
    double sum1;
    double sum2;
    double previousPacketTime;
    double oneMoreHour;
    double computedSkew;
    
    unsigned long long int preliminaryNumOfPackets;
    double preliminarySum1;
    double preliminarySum2;
    double preliminaryAverage;

    // Public attributes
  public:
    /// FIXME - comment needed
    bool firstPacketReceived;

  // Constructors
  public:
    ComputerInfo(void * parentList, const char* its_address, u_int16_t port);

    ~ComputerInfo();

  // Public methods
  public:
    const std::string& get_address() const
    {
      return address;
    }
    
    const std::string& get_ipAddress() const
    {
      return ipAddress;
    }
    
    uint16_t get_port() const
    {
      return port;
    }

    int get_freq() const
    {
      return freq;
    }

    unsigned long get_packets_count() const
    {
      return packets.size();
    }

    double get_last_packet_time() const
    {
      return lastPacketTime;
    }

    double get_start_time() const
    {
      return startTime;
    }

    uint64_t get_last_packet_timestamp() const
    {
      return packets.rbegin()->Timestamp;
    }

    // TODO: why not confirmedSkew.Alpha or last TimeSegment from TimeSegmentList ?
    const PacketSegment& get_last_packet_segment() const
    {
      return *(packetSegmentList.rbegin());
    }
    
    TimeSegmentList timeSegmentList;
    //
    TimeSegmentList NewTimeSegmentList;
      
    /**
     * Adds a new packet without updating or recomputing anything
     * @param[in] packet_delivered       Arrival time of the new packet
     * @param[in] timestamp              TCP timestamp of the new packet
     */
    void insert_packet(double packet_delivered, uint64_t timestamp);
    
    void insert_first_packet(double packet_delivered, uint64_t timestamp);

    /**
     * Recomputes related informations
     * @param[in] packet_delivered      Arrival time of the new packet
     * @return Returns if a block processing already finished
     */
    bool check_block_finish(double packet_delivered);

    /**
     * Restart measurement
     * @param[in] packet_delivered      Arrival time of the first packet
     * @param[in] timestamp             TCP timestamp of the first packet
     */
    void restart(double packet_delivered, uint64_t timestamp);
    
    /// Reduces unnecessary information about packets
    void reduce_packets(packet_iterator start, packet_iterator end);

    /** 
     * Save packets into file (called 'IP address.log')
     * @param[in] rewrite   Boolean that conrols if the file is overwritten or the packets are appended
     * @return 0            if ok
     * */
    int save_packets(short int rewrite) const;

  private:
    /// Performs actions after a block of packets is captured
    void recompute_block(double packet_delivered);
    /// Adds initialized empty skew information
    void add_empty_packet_segment(packetTimeInfoList::iterator start);
    /// Computes a new skew
    ClockSkewPair compute_skew(const packet_iterator &start, const packet_iterator &end);
    /// Computes a new frequency
    int compute_freq();

		/// Outputs summary results of clock skew computed per packet
		void output_skewbypacket_results(double skew);
};

#endif
