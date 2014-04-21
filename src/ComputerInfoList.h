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

#ifndef _COMPUTER_INFO_LIST_H
#define _COMPUTER_INFO_LIST_H

#include "AnalysisInfo.h"
#include "Observer.h"
#include "Observable.h"
#include "ComputerInfo.h"
#include "gnuplot_graph.h"

/**
 * All informations known about a set of computers.
 */
class ComputerInfoList : public Observable<const AnalysisInfo> {
  // Attributes
  private:
    /// Informations about packet timing
    std::list<ComputerInfo *> computers;
    /// Informations about clock skew
    // clock_skew_guard skews;
    /// Last time when inactive computers were detected
    double last_inactive;
    std::string type;
    
    void construct_notify(const std::string &ip, const identity_container &identitites, const TimeSegmentList &s) const;
    void construct_notify(const std::string &ip) const;
    
    TimeSegmentList * getSkew(std::string ip);

  // Constructors
  public:
    // create graph instead of using observers
    gnuplot_graph * graph_creator;
    ComputerInfoList(std::string type): last_inactive(time(NULL)), type(type), lastXMLupdate(0)
    {}
    
    ~ComputerInfoList();

    std::string getOutputDirectory(){
        return type + "/";
    }
    
    /**
     * New packet processing (classify, save, compute...)
     * @param[in] address IP address of the source
     * @param[in] time Real time when packet arrived
     * @param[in] timestamp PCAP timestamp of the packet
     * @return 0 if new IP address, 1 if computer already known
     */
    bool new_packet(const char *address, u_int16_t port, double time, uint32_t timestamp);

    /**
     * Registers a new observer for clock changes
     *
     * @param[in] obs The observer to be added
     */
    /*void add_observer(observer<const computer_skew>* obs)
    {
      this->add_observer(obs);
    }*/
    
    /**
     * Adds or updates clock skew value of a address
     * @param[in] ip The IP address for which the clock skew is provided
     * @param[in] skew Clock skew of the IP address
     */
    void update_skew(const std::string &ip, const TimeSegmentList &s);

    /**
     * Returns IP addresses with similar clock skew to the IP address provided
     * @param[in] ip The IP whose clock skew will be compared
     */
    const identity_container get_similar_identities(const std::string &ip);

    void to_poke_or_not_to_poke(std::string address);
    time_t lastXMLupdate;
};

#endif
