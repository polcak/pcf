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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <iostream>
#include <fstream>

#include "capture.h"
#include "check_computers.h"
#include "Configurator.h"

/**
 * Print banner
 */
void print_banner() {
  printf( "pcf  Copyright (C) 2012-2013 Jakub Jirasek <xjiras02@stud.fit.vutbr.cz>\n"
          "                             Libor Polcak <ipolcak@fit.vutbr.cz>\n"
          "                             Barbora Frankova <xfrank08@stud.fit.vutbr.cz>\n\n"
          "This program comes with ABSOLUTELY NO WARRANTY.\n"
          "This is free software, and you are welcome to redistribute it\n"
          "under certain conditions.\n\n");
}

/**
 * Print help
 */
void print_help() {
  printf("Usage: pcf [Options] [Interface]\n\n"
          "  -h\t\tPrint this help\n"
          "  -n\t\tNumber of packets to capture (0 for infinity)\n"
          "  -t\t\tTime for capturing (in seconds, 0 for infinity)\n"
          "  -p\t\tPort number (1-65535)\n"
          "  -i\t\tDisable ICMP\n"
          "  -j\t\tDisable Javascript\n"
          "  -x\t\tDisable TCP\n"
          "  -d\t\tPair devices using port numbers, e.g. to detect devices behind NAT\n"
          "  -o filename\tRead from pcap file\n"
          "  -v\t\tVerbose mode\n"
          "  -r\t\tReduce packets\n"
          "  -e\t\tIRI-IIF outputs\n"
          "  -f\t\tSet frequency and recompute skew after every packet (use ONLY with ONE IP at a time)\n"
          "Examples:\n"
          "  pcf\n"
          "  pcf -n 100 -t 600 -p 80 wlan0\n\n");
}

/**
 * Main
 */

int main(int argc, char *argv[]) {
  /// Checking permissions (must be root)
  if (getuid()) {
    fprintf(stderr, "Must have root permissions to run this program!\n");
    return (2);
  }

  /// Get config
  char filename[] = "config";
  Configurator::instance()->GetConfig(filename);

  /// Get params
  int c;
  opterr = 0;
  while ((c = getopt(argc, argv, "ivhbxes:f:n:t:p:jdo:r")) != -1) {
    switch (c) {
      case('i'):
        Configurator::instance()->icmpDisable = true;
        break;
      case('j'):
        Configurator::instance()->javacriptDisable = true;
        break;
      case('x'):
        Configurator::instance()->tcpDisable = true;
        break;
      case('f'):
        if (atof(optarg))
          Configurator::instance()->setFreq = atof(optarg);
        else
          fprintf(stderr, "Wrong frequency\n");
        break;
      case('s'):
        if (atof(optarg))
          Configurator::instance()->setSkew = atof(optarg);
        else
          fprintf(stderr, "Wrong skew\n");
        break;
      case('b'):
        Configurator::instance()->bashOutput = true;
        break;
      case('d'):
        Configurator::instance()->portEnable = true;
        break;
      case 'v':
        Configurator::instance()->verbose = true;
        break;
      case 'h':
        print_help();
        return (0);
      case 'e':
        Configurator::instance()->exportSkewChanges = true;
        break;
      case 'n':
        if (atoi(optarg))
          Configurator::instance()->number = atoi(optarg);
        else
          fprintf(stderr, "Wrong number of packets\n");
        break;
      case 't':
        if (atoi(optarg))
          Configurator::instance()->time = atoi(optarg);
        else
          fprintf(stderr, "Wrong time for capturing\n");
        break;
      case 'p':
        if (atoi(optarg) && (atoi(optarg) < 65535))
          Configurator::instance()->port = atoi(optarg);
        else
          fprintf(stderr, "Wrong port number\n");
        break;
      case 'o':
          Configurator::instance()->datafile = optarg;
        break;
      case 'r':
          Configurator::instance()->reduce = true;
        break;
    }
  }
  c = optind;
  if (c < argc) {
    if (strlen(argv[c]) < 10)
      strcpy(Configurator::instance()->dev, argv[c]);
    else {
      fprintf(stderr, "Device name too long (%s)\nSetting to any", argv[c]);
      strcpy(Configurator::instance()->dev, "any");
    }
  }

  /// Print banner
  if (Configurator::instance()->verbose) {
    print_banner();
  }

  /// Get packets
  if (StartCapturing() != 0)
    return (2);

  return (0);
}
