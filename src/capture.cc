/**
 * Copyright (C) 2012 Jakub Jirasek <xjiras02@stud.fit.vutbr.cz>
 *                    Libor Polčák <ipolcak@fit.vutbr.cz>
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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <string>
#include <regex>

#include <pcap.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>
#include <netinet/ip_icmp.h>
#include <pthread.h>
#include <netinet/in_systm.h>

#include "capture.h"
#include "ComputerInfoList.h"
#include "gnuplot_graph.h"
#include "Configurator.h"
#include "Tools.h"
#include "ComputerInfoIcmp.h"
#include "SkewChangeExporter.h"

/// Capture all packets on the wire
#define PROMISC 1
#define LIN_COOK_SIZE 16

/// Pcap session handle
pcap_t *handle;

// IPv6 addr length (39B) + '\0' + some padding
#define ADDRESS_SIZE 64

ComputerInfoList * computersTcp;
ComputerInfoList * computersIcmp;
ComputerInfoList * computersJavascript;

void StopCapturing(int signum) {
  pcap_breakloop(handle);
}

void GotPacket(u_char *args, const struct pcap_pkthdr *header, const u_char *packet) {
  // Allocate space for an address
  char address[ADDRESS_SIZE];
  // port
  u_int16_t port = 0;
  // TCP header
  const struct tcphdr *tcp = NULL;
  // ICMP header
  const struct icmphdr *icmp = NULL;
  // Packet arrival time (us)
  double arrival_time;
  // Timestamp
  uint32_t timestamp;
  int pokeOk = true;
  //
  std::string type = "";

  // number of processed packets
  static int n_packets = 0;

  // Sizes
  int size_link_proto;
  int size_ip;
  int size_tcp;
  bool newIp = false;
  u_int16_t type_link_proto;

  // check physical layer protocol
  if(Configurator::instance()->datalink == "EN10MB"){
    // Ethernet header
    const struct ether_header * ether = (struct ether_header*) packet;
    /// Ethernet
    size_link_proto = sizeof (struct ether_header);
    type_link_proto = ntohs(ether->ether_type);
    
  }
  else if(Configurator::instance()->datalink == "LINUX_SLL"){
    // Linux Cooked header
    linux_cooked_hdr * lin_cook = (linux_cooked_hdr*) packet;
    size_link_proto = LIN_COOK_SIZE;
    type_link_proto = ntohs(lin_cook->proto_type);
  }
  else {
    std::cerr<< "Unknown physical layer protocol." << std::endl;
    return;
  }
  
  // VLAN
  if(type_link_proto == 0x8100){
    type_link_proto = ntohs(*((uint16_t *)(packet + size_link_proto) + 1));
    size_link_proto += 4;
  }

  /// IPv4
  if (type_link_proto == 0x0800) {
    // IP header
    const struct ip *ip = (struct ip*) (packet + size_link_proto);
    size_ip = sizeof (struct ip);
    // Check if the packet is ICMP or TCP
    if (ip->ip_p == IPPROTO_ICMP) {
      type = "icmp";
      icmp = (struct icmphdr*) (packet + size_link_proto + size_ip);
    } else if (ip->ip_p == IPPROTO_TCP) {
      type = "tcp";
      tcp = (struct tcphdr*) (packet + size_link_proto + size_ip);
    } else
      return;

    // get IP address
    if (inet_ntop(AF_INET, &(ip->ip_src), address, 64) == NULL) {
      fprintf(stderr, "Cannot get IP address\n");
      return;
    }
  }    /// IPv6
  else if (type_link_proto == 0x86dd) {
    // IP header
    pokeOk = false;
    const struct ip6_hdr *ip = (struct ip6_hdr*) (packet + size_link_proto);
    size_ip = sizeof (struct ip6_hdr);
    /// Check if the packet is TCP
    if (ip->ip6_ctlun.ip6_un1.ip6_un1_nxt != IPPROTO_TCP)
      return;
    type = "tcp";
    /// TCP
    tcp = (struct tcphdr*) (packet + size_link_proto + size_ip);
    if (inet_ntop(AF_INET6, &(ip->ip6_src), address, 64) == NULL) {
      fprintf(stderr, "Cannot get IP address\n");
      return;
    }
  } else {
    fprintf(stderr, "Unknown Ethernet type\n");
    return;
  }

  if (type == "tcp") {
    port = ntohs(tcp->source);
    // skip TCP headers
    size_tcp = tcp->doff * 4;
    if (size_tcp < 20) {
#ifdef DEBUG
      std::cerr << "Invalid TCP header length: " << size_tcp << " bytes" << std::endl;
#endif
      return;
    }
    
    /// Packet arrival timearrival_time
    arrival_time = header->ts.tv_sec + (header->ts.tv_usec / 1000000.0);

    // Packets with 20 bytes are without TCP options, they are processed because of possible
    // timestamps in the payload
    if (size_tcp == 20) {
#ifdef DEBUG
      std::cerr << "TCP header without options" << std::endl;
#endif
    }
    /// TCP options
    u_char *tcp_options = (u_char *) (packet + size_link_proto + size_ip + sizeof (struct tcphdr));

    int options_size = size_tcp - 20;
    int options_offset = 0;

    // TCP options
    // +--------+--------+---------+--------+
    // |  Kind  | Length |       Data       |
    // +--------+--------+---------+--------+

    while (options_offset < options_size) {
      if(Configurator::instance()->tcpDisable){
        break;
      }
      int kind = (int) tcp_options[options_offset];
      int option_len = 0;

      if (kind == 8) {
        timestamp = ntohl(*((uint32_t*) (&tcp_options[options_offset + 2])));

        /// Save packet
        n_packets++;
        newIp = !computersTcp->new_packet(address, port, arrival_time, timestamp);
        if (Configurator::instance()->verbose) {
          if(Configurator::instance()->portEnable)
            std::cout << n_packets << ": " << address << "_" << port << " (TCP)" << std::endl;
          else
            std::cout << n_packets << ": " << address << " (TCP)" << std::endl;
        }
        if (newIp) {
          if (pokeOk && !Configurator::instance()->icmpDisable)
            computersIcmp->to_poke_or_not_to_poke(address);
        }
      }

      switch (kind) {
          /// EOL
        case 0:
#ifdef DEBUG
          std::cerr << "TCP header option without timestamp" << std::endl;
#endif
          options_offset = options_size;
          break;
          /// NOP
        case 1:
          options_offset++;
          break;
        default:
          option_len = (int) tcp_options[options_offset + 1];
          options_offset += option_len;
          break;
      }
    }
    
    if(Configurator::instance()->javacriptDisable){
      return;
    }
    
    // parse HTTP header
    int remaining_length = header->len - (size_link_proto + size_ip + size_tcp);
    // TCP without HTTP
    if (remaining_length == 0)
      return;

    char * hl = (char*) tcp + size_tcp;
    std::string httpRequest = "";
    httpRequest.append(hl, remaining_length);

    // find timestamp
    std::string ts = "";
    int found = httpRequest.find("ts=");
    if (found < 0)
      return;
    // check if enough data to read
    if (found + 16 > (int) httpRequest.length())
      return;
    ts = httpRequest.substr(found + 3, 13);
    // convert timestamp
    long long longTimestamp = atoll(ts.c_str());
    // convert UNIX timestamp to h:m:s:ms
    longTimestamp = longTimestamp % (3600 * 24 * 1000);
    // save new packet
    computersJavascript->new_packet(address, port, arrival_time, longTimestamp);
    if (Configurator::instance()->verbose) {
      if(Configurator::instance()->portEnable)
        std::cout << n_packets << ": " << address << "_" << port << " (JS)" << std::endl;
      else
        std::cout << n_packets << ": " << address << " (JS)" << std::endl;
    }
    return; // Packet processed


  } else if (type == "icmp") {
    // ICMP header
    // +--------+--------+---------+--------+
    // |  Type  |  Code  |  Header Chcksum  |
    // +--------+--------+---------+--------+
    // |    Identifier   |   Sequence Num   |
    // +--------+--------+---------+--------+
    // |        Originate Timestamp         |
    // +--------+--------+---------+--------+
    // |         Recieve Timestamp          |
    // +--------+--------+---------+--------+
    // |        Transmit Timestamp          |
    // +--------+--------+---------+--------+

    // packet is not ICMP timestamp reply -> throw away
    if (icmp->type != ICMP_TSTAMPREPLY)
      return;
    // retrieve appropriate timestamp
    unsigned int * newTimestamp = (unsigned int *) icmp + 3;
    timestamp = (uint32_t) ntohl(*newTimestamp);
    arrival_time = header->ts.tv_sec + (header->ts.tv_usec / 1000000.0);
    // save packet 
    computersIcmp->new_packet(address, 0, arrival_time, timestamp);
    if (Configurator::instance()->verbose) {
      std::cout << n_packets << ": " << address << " (ICMP)" << std::endl;
    }
    // packet processed
    return;
  }
}

int liveCapturing(){
  /// Checking permissions (must be root)
  if (getuid()) {
    fprintf(stderr, "Must have root permissions to run this program!\n");
    return (2);
  }
  // Dot notation network address
  char *net;
  // Dot notation network mask
  char *mask;
  // Error string
  char errbuf[PCAP_ERRBUF_SIZE];
  // Netmask of sniffing device
  bpf_u_int32 maskp;
  // IP or sniffing device
  bpf_u_int32 netp;
  // Addresses
  struct in_addr addr;

  /// Set the device
  char *dev = Configurator::instance()->dev;
  if (strcmp(dev, "") == 0) {
    dev = pcap_lookupdev(errbuf);
    if (dev == NULL) {
      fprintf(stderr, "Couldn't find default device: %s\n", errbuf);
      return (2);
    }
  }

#ifdef DEBUG
  std::cout << "DEV: " << dev << std::endl;
#endif

  /// Find address and netmask for the device
  if (pcap_lookupnet(dev, &netp, &maskp, errbuf) == 0) {
    addr.s_addr = netp;
    net = inet_ntoa(addr);
    if (net == NULL)
      std::cerr << "Can't convert net address" << std::endl;
    else {
#ifdef DEBUG
      std::cout << "NET: " << net << std::endl;
#endif
    }

    /// Netmask
    addr.s_addr = maskp;
    mask = inet_ntoa(addr);
    if (mask == NULL)
      std::cerr << "Can't convert net mask" << std::endl;
    else {
#ifdef DEBUG
      std::cout << "MASK: " << mask << std::endl;
#endif
    }
  } else
    std::cerr << "(Can't get netmask for device: " << dev << std::endl;

  handle = pcap_open_live(dev, BUFSIZ, PROMISC, 1000, errbuf);
    if (handle == NULL) {
      std::cerr << "Couldn't open device" << dev << ": " << errbuf << std::endl;
      return (2);
 }
  return 0;
}

int offlineCapturing(){
  // Error string
  char errbuf[PCAP_ERRBUF_SIZE];
  handle = pcap_open_offline(Configurator::instance()->datafile.c_str(), errbuf);
    if (handle == NULL) {
      std::cerr << "Couldn't open file" << Configurator::instance()->datafile << ": " << errbuf << std::endl;
      return (2);
    } 
  return 0;
}

int StartCapturing() {
  // Filter expr.
  std::string filter;
  // Compiled filter expr.
  struct bpf_program fp;
  /// Open the device for sniffing
  if(Configurator::instance()->datafile.empty()){
    if(liveCapturing()){
      return(2);
    }
  }
  // Open offline pcap file
  else {
    if(offlineCapturing()){
      return(2);
    }
  }

  // TCP
  filter = "(tcp";
  // Port
  if (Configurator::instance()->port != 0) {
    filter += " && port " + Tools::IntToString(Configurator::instance()->port);
  }
  // Src
  if (strcmp(Configurator::instance()->src, "") != 0) {
    filter += " && src ";
    filter += Configurator::instance()->src;
  }
  // Dst
  if (strcmp(Configurator::instance()->dst, "") != 0) {
    filter += " && dst host ";
    filter += Configurator::instance()->dst;
  }
  // SYN
  if (Configurator::instance()->syn == 1) {
    filter += " && tcp[tcpflags] & tcp-syn == tcp-syn ";
  }
  // ACK	
  if (Configurator::instance()->ack == 1) {
    filter += " && tcp[tcpflags] & tcp-ack == tcp-ack ";
  }
  // Filter
  if (strcmp(Configurator::instance()->filter, "") != 0) {
    filter += " && (";
    filter += Configurator::instance()->filter;
    filter += ")";
  }

  filter += ") || (icmp && icmp[icmptype] == icmp-tstampreply)";
  filter += " || (vlan)";

#ifdef DEBUG
  std::cout << "Filter: " << filter << std::endl;
#endif

  if (pcap_compile(handle, &fp, filter.c_str(), 0, PCAP_NETMASK_UNKNOWN) == -1) {
    std::cerr << "Couldn't parse filter " << filter << ": " << pcap_geterr(handle) << std::endl;
    return (2);
  }

  /// Apply the filter
  if (pcap_setfilter(handle, &fp) == -1) {
    std::cerr << "Couldn't install filter " << filter << ": " << pcap_geterr(handle) << std::endl;
    return (2);
  }

  /// Set alarm (if any)
  if (Configurator::instance()->time > 0) {
    signal(SIGALRM, StopCapturing);
    alarm(Configurator::instance()->time);
  }

  computersTcp = new ComputerInfoList("tcp");
  computersJavascript = new ComputerInfoList("javascript");
  computersIcmp = new ComputerInfoList("icmp");
  // ComputerInfoList computers(Configurator::instance()->active, Configurator::instance()->database, Configurator::instance()->block, Configurator::instance()->timeLimit, Configurator::instance()->threshold);
  
  gnuplot_graph graph_creator_tcp("tcp");
  gnuplot_graph graph_creator_javascript("javascript");
  gnuplot_graph graph_creator_icmp("icmp");
  
  computersTcp->AddObserver(&graph_creator_tcp);
  computersIcmp->AddObserver(&graph_creator_icmp);
  computersJavascript->AddObserver(&graph_creator_javascript);
  
  if (Configurator::instance()->exportSkewChanges) {
    computersTcp->AddObserver(new SkewChangeExporter("tcp"));
    computersIcmp->AddObserver(new SkewChangeExporter("icmp"));
    computersJavascript->AddObserver(new SkewChangeExporter("javascript"));
  }

  /// Set interrupt signal (ctrl-c or SIGTERM during capturing means stop capturing)
  struct sigaction sigact;
  memset(&sigact, 0, sizeof (sigact));
  sigact.sa_handler = StopCapturing;
  sigemptyset(&sigact.sa_mask);
  sigact.sa_flags = SA_SIGINFO;
  if (sigaction(SIGINT, &sigact, NULL) != 0) {
    std::cerr << "Couldn't set SIGINT" << std::endl;
    return (2);
  }
  if (sigaction(SIGTERM, &sigact, NULL) != 0) {
    std::cerr << "Couldn't set SIGTERM" << std::endl;
    return (2);
  }
  struct sigaction sigact2;
  memset(&sigact2, 0, sizeof (sigact2));
  sigact2.sa_handler = SIG_IGN;
  if (sigaction(SIGHUP, &sigact2, NULL) != 0) {
    std::cerr << "Couldn't set SIGHUP" << std::endl;
    return (2);
  }

  /// Print actual time
  if (Configurator::instance()->verbose) {
    time_t rawtime;
    time(&rawtime);
    std::cout << "Capturing started at: " << ctime(&rawtime) << std::endl;
  }
  
  Configurator::instance()->datalink = pcap_datalink_val_to_name(pcap_datalink(handle));

  /// Start capturing TODO
  if (pcap_loop(handle, Configurator::instance()->number, GotPacket, NULL) == -1) {
    std::cerr << "An error occured during capturing: " << pcap_geterr(handle) << std::endl;
    return (2);
  }

  if (!Configurator::instance()->tcpDisable) {
    computersTcp->save_active_computers();
  }
  if (!Configurator::instance()->javacriptDisable) {
    computersJavascript->save_active_computers();
  }
  if (!Configurator::instance()->icmpDisable) {
    computersIcmp->save_active_computers();
  }

  /// Close the session
  pcap_close(handle);
  return (0);
}
