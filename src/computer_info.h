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

#ifndef _COMPUTER_INFO_H
#define _COMPUTER_INFO_H

#include <list>
#include <string>
#include <utility>

#include "clock_skew_guard.h"
#include "packet_time_info.h"


/**
 * Typedef for a list of all packets known for one computer.
 */
typedef std::list<packet_time_info> packet_time_info_list;
typedef packet_time_info_list::iterator packet_iterator;

/**
 * Clock skew pair (alpha, beta): y = alpha*x + beta
 */
typedef std::pair<double, double> clock_skew_pair;

/**
 * All informations known about each computer including time information about all received packets.
 */
class computer_info {
  private:
    /// Structure describing clock skew
    struct skew_info {
      double alpha;
      double beta;
      packet_iterator first;
      packet_iterator confirmed;
      packet_iterator last;
    };

  // Attributes
  private:

    /// List of time informations about packets
    packet_time_info_list packets;

    /// Address of the computer
    std::string address;

    /// Frequency of the computer
    int freq;

    /// Time of the last added packet
    double last_packet_time;

    /// Time of the packet with last confirmed skew
    double last_confirmed_skew;

    /// Last confirmed skew
    clock_skew_pair confirmed_skew;

    /// Time of the start of the capture
    double start_time;

    /// Skew information about one computer
    std::list<skew_info> skew_list;

    /**
     * Number of packets in one block
     * Some operations are performed only for each block or every nth block
     */
    const int block_size;

  // Constructors
  public:
    computer_info(double first_packet_delivered, uint32_t first_packet_timstamp,
        const char* its_address, const int its_block_size);

  // Destructor
  public:
    ~computer_info() {}

  // Getters
  public:
    const std::string& get_address() const
    {
      return address;
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
      return last_packet_time;
    }

    double get_start_time() const
    {
      return start_time;
    }

    uint32_t get_last_packet_timestamp() const
    {
      return packets.rbegin()->timestamp;
    }

    const skew_info& get_skew() const
    {
      return *(skew_list.rbegin());
    }

  // Functions manipulating the state
  public:
    /**
     * Adds a new packet without updating or recomputing anything
     * @param[in] packet_delivered Arrival time of the new packet
     * @param[in] timestamp TCP timestamp of the new packet
     * @param[inout] skews Storage of clock skews of known devices
     */
    void insert_packet(double packet_delivered, uint32_t timestamp);

    /**
     * Adds a new packet and possibly recomputes related informations
     * @param[in] packet_delivered Arrival time of the new packet
     * @param[in] timestamp TCP timestamp of the new packet
     * @param[inout] skews Storage of clock skews of known devices
     */
    void insert_packet(double packet_delivered, uint32_t timestamp, clock_skew_guard &skews);

    /**
     * Restart measurement
     * @param[in] packet_delivered Arrival time of the first packet
     * @param[in] timestamp TCP timestamp of the first packet
     */
    void restart(double packet_delivered, uint32_t timestamp);
    /// Reduces unnecessary information about packets
    void reduce_packets(packet_iterator start, packet_iterator end);

  private:
    ///Performs actions after a block of packets is captured
    void block_finished(double packet_delivered, clock_skew_guard &skews);
    /// Adds initialized empty skew information
    void add_empty_skew(packet_time_info_list::iterator start);
    /// Computes a new skew
    clock_skew_pair compute_skew(const packet_iterator &start, const packet_iterator &end);
    /// Computes a new frequency
    int compute_freq();
    /** 
     * Save packets into file (called 'IP address.log')
     * @param[in] rewrite Boolean that conrols if the file is overwritten or the packets are appended
     * @return 0 if ok
     * */
    int save_packets(short int rewrite);
    /// Generate gnuplot command file
    void generate_graph(const clock_skew_pair &skew, clock_skew_guard &skews);

};

#endif
