/**
 * Copyright (C) 2012 Jakub Jirasek <xjiras02@stud.fit.vutbr.cz>
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

#include <pcap.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

#include "capture.h"
#include "list.h"


/// Pcap session handle
pcap_t *handle;


/**
 * Signal handler to stop capturing
 */
void stop_capturing(int signum);

/**
 * Get Actual time
 * @return Actual time in seconds
 */
//double get_time();

/** 
 * Callback function for pcap
 * @param[in] args User params
 * @param[in] header Packet header
 * @param[in] packet Packet
 */
void got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet);

/**
 * Making filter string
 * @param[in,out] filter String with filter
 * @param[in] add String to add
 * @param[in,out] length Filter string length
 * @return 0 if ok
 */
int add_to_filter(char *filter, const char *add, int *length);


void stop_capturing(int signum)
{
  pcap_breakloop(handle);
}

/*
double get_time()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  double result = tv.tv_sec + (tv.tv_usec / 1000000.0);
  
  return result;
}
*/

int add_to_filter(char *filter, const char *add, int *length)
{
  char *tmp = (char*)realloc(filter, strlen(filter) + strlen(add) + sizeof(char));
  if (tmp == NULL)
    return(1);
  
  filter = tmp;
  strncat(filter, add, strlen(add));
  *length += strlen(add);
  
  return(0);
}

void got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet)
{
  // IP header
  const struct ip *ip;
  // TCP header
  const struct tcphdr *tcp;
  // Packet arrival time (us)
  double arrival_time;
  // Timestamp
  unsigned long int timestamp;
  
  // Sizes
  int size_ethernet;
  int size_ip;
  int size_tcp;
  
  /// Ethernet
  size_ethernet = sizeof(struct ether_header);
  
  /// IP
  ip = (struct ip*)(packet + size_ethernet);
  size_ip = sizeof(struct ip);
  /// Check if the packet is TCP
  if (ip->ip_p != IPPROTO_TCP)
    return;
  
  /// TCP
  tcp = (struct tcphdr*)(packet + size_ethernet + size_ip);
  size_tcp = tcp->doff*4;
  if (size_tcp < 20) {
#ifdef DEBUG
    fprintf(stderr, "Invalid TCP header length: %u bytes\n", size_tcp);
#endif
    return;
  }
  
  // For sure, should filter pcap
  if (size_tcp == 20) {
#ifdef DEBUG
    printf("TCP header without options\n");
#endif
    return;
  }
  /// TCP options
  u_char *txt = (u_char *)(packet + size_ethernet + size_ip + sizeof(struct tcphdr));
  
  // TCP options
  // +--------+--------+---------+--------+
  // |  Kind  | Length |       Data       |
  // +--------+--------+---------+--------+

  int kind = (int)txt[0];

  while (kind != 8) {

    switch(kind) {

      /// EOL
      case 0:
#ifdef DEBUG
        printf("TCP header option without timestamp\n");
#endif
        return;

      /// NOP
      case 1:
        txt++;
        kind = (int)txt[0];
        break;

      default:
        txt++;
        kind = (int)txt[0];
        txt += kind - 1;
        kind = (int)txt[0];
        break;
    }
  }

  /// Skip length
  txt += 2;
  int i;
  char c[10];
  c[0] = '0';
  c[1] = 'x';
  char *tmp = c;
  tmp += 2;
  for(i = 0; i < 4; i++) {
    sprintf(tmp, "%02x", *txt);
    tmp += 2;
    txt++;
  }  
  
  /// Timestamp
  timestamp = strtoul(c, NULL, 16);
  /// Packet arrival time
  arrival_time = header->ts.tv_sec + (header->ts.tv_usec / 1000000.0);
  
  /// Save packet
  new_packet(inet_ntoa(ip->ip_src), arrival_time, timestamp);
}


int capture(my_config *config)
{
  // Dot notation network address
  char *net;
  // Dot notation network mask
  char *mask;
  // Error string
  char errbuf[PCAP_ERRBUF_SIZE];
  // Compiled filter expr.
  struct bpf_program fp;
  // Filter expr.
  char *filter = NULL;
  // Netmask of sniffing device
  bpf_u_int32 maskp;
  // IP or sniffing device
  bpf_u_int32 netp;
  // Addresses
  struct in_addr addr;

  
  /// Set the device
  char *dev = config->dev;
  if (strcmp(dev, "") == 0) {
    dev = pcap_lookupdev(errbuf);
    if (dev == NULL) {
      fprintf(stderr, "Couldn't find default device: %s\n", errbuf);
      return(2);
    }
  }
  
#ifdef DEBUG
  /// Print selected device
  printf("DEV: %s\n", dev);
#endif
  
  /// Find address and netmask for the device
  if (pcap_lookupnet(dev, &netp, &maskp, errbuf) == 0) {
    addr.s_addr = netp;
    net = inet_ntoa(addr);
    if (net == NULL)
      fprintf(stderr, "Can't convert net address\n");
#ifdef DEBUG
    else
      printf("NET: %s\n", net);
#endif
  
    /// Netmask
    addr.s_addr = maskp;
    mask = inet_ntoa(addr);
    if (mask == NULL)
      fprintf(stderr, "Can't convert net mask\n");
#ifdef DEBUG
    else
      printf("MASK: %s\n\n", mask);
#endif
  }
  else
    fprintf(stderr, "(Can't get netmask for device: %s)\n\n", dev);
  
  /// Open the device for sniffing
  handle = pcap_open_live(dev, BUFSIZ, 0, 1000, errbuf);
  if (handle == NULL) {
    fprintf(stderr, "Couldn't open device %s: %s\n", dev, errbuf);
    return(2);
  }
  
  /// Compile the filter
  char tmp[10];
  int length = 0;
  
  // TCP
  filter = (char*)realloc(filter, strlen("tcp"));
  strcpy(filter, "tcp");
  length += strlen("tcp");
  // TCP header with options and (very likely) with timestamps
  add_to_filter(filter, " && tcp[12] >= 120 ", &length);
  // Port
  if (config->port != 0) {
    add_to_filter(filter, " && port ", &length);
    sprintf(tmp, "%d", config->port);
    add_to_filter(filter, tmp, &length);
  }
  // Src
  if (strcmp(config->src, "") != 0) {
    add_to_filter(filter, " && src host ", &length);
    add_to_filter(filter, config->src, &length);
  }
  // Dst
  if (strcmp(config->dst, "") != 0) {
    add_to_filter(filter, " && dst host ", &length);
    add_to_filter(filter, config->dst, &length);
  }
  // SYN
  if (config->syn == 1) {
    add_to_filter(filter, " && tcp[tcpflags] & tcp-syn == tcp-syn ", &length);
  }
  // ACK	
  if (config->ack == 1) {
    add_to_filter(filter, " && tcp[tcpflags] & tcp-ack == tcp-ack ", &length);
  }
  // Filter
  if (strcmp(config->filter, "") != 0) {
    add_to_filter(filter, " && ", &length);
    add_to_filter(filter, config->filter, &length);
  }
  filter[length] = '\0';
  
#ifdef DEBUG
  /// Print the filter
  printf("Filter: %s\n", filter);
#endif
  
  if (pcap_compile(handle, &fp, filter, 0, PCAP_NETMASK_UNKNOWN) == -1) {
    fprintf(stderr, "Couldn't parse filter %s: %s\n", filter, pcap_geterr(handle));
    return(2);
  }
  
  /// Apply the filter
  if (pcap_setfilter(handle, &fp) == -1) {
    fprintf(stderr, "Couldn't install filter %s: %s\n", filter, pcap_geterr(handle));
    return(2);
  }
  
  /// Free the filter
  free(filter);
  
  /// Set alarm (if any)
  if (config->time > 0) {
    signal(SIGALRM, stop_capturing);
    alarm(config->time);
  }
  
  /// Set active
  active = config->active;
  
  /// Set database
  database = config->database;
  
  /// Set BLOCK
  BLOCK = config->block;
  
  /// Set TIME_LIMIT
  TIME_LIMIT = config->time_limit;
  
  /// Set THRESHOLD
  THRESHOLD = config->threshold;
  
  /// Set interrupt signal (ctrl-c or SIGTERM during capturing means stop capturing)
  struct sigaction sigact;
  memset(&sigact, 0, sizeof(sigact));
  sigact.sa_handler = stop_capturing;
  sigemptyset(&sigact.sa_mask);
  sigact.sa_flags = SA_SIGINFO;
  if (sigaction(SIGINT, &sigact, NULL) != 0) {
    fprintf(stderr, "Couldn't set SIGINT\n");
    return(2);
  }
  if (sigaction(SIGTERM, &sigact, NULL) != 0) {
    fprintf(stderr, "Couldn't set SIGTERM\n");
    return(2);
  }
  struct sigaction sigact2;
  memset(&sigact2, 0, sizeof(sigact2));
  sigact2.sa_handler = SIG_IGN;
  if (sigaction(SIGHUP, &sigact2, NULL) != 0) {
    fprintf(stderr, "Couldn't set SIGHUP\n");
    return(2);
  }
  
  /// Print actual time
  time_t rawtime;
  time(&rawtime);
  printf("Capturing started at: %s\n", ctime(&rawtime));
  
  /// Start capturing
  if (pcap_loop(handle, config->number, got_packet, NULL) == -1) {
    fprintf(stderr, "An error occured during capturing: %s\n", pcap_geterr(handle));
    return(2);
  }
  
  /// Close the session
  pcap_close(handle);
  
  return(0);
}
