/**
 * Copyright (C) 2013 Libor Polčák <ipolcak@fit.vutbr.cz>
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

#include<cstring>
#include<iostream>
#include<fstream>

#include "parse_config.h"
#include "computer_info_list.h"
#include "gnuplot_graph.h"

/**
 * Print help
 */
void print_help()
{
  printf("Usage: log_reader file\n\n"
         "  -h\t\tPrint this help\n"
         "  file Name of the file to be parsed\n"
         "Examples:\n"
         "  log_reader log/192.168.1.1\n\n");
}


/**
 * Read the content of the log file and process it
 */
void process_log_file(std::ifstream &ifs, pcf_config *config)
{
  computer_info_list computers(config->active, config->database, config->block, config->time_limit, config->threshold);
  gnuplot_graph graph_creator;
  computers.add_observer(&graph_creator);

  double ttime, offset;

  while (ifs.good()) {
    ifs >> ttime >> offset;
    if (ifs.good()) {
      computers.new_packet("log_reader", ttime, ttime + offset);
    }
  }
}

/**
 * Main
 */

int main(int argc, char *argv[])
{
  if ((argc < 2) || (std::strcmp(argv[1], "-h") == 0)) {
    print_help();
    return 1;
  }

  // Get config
  char filename[] = "config";
  pcf_config *config = get_config(filename);
  if (config == NULL) {
    fprintf(stderr, "Cannot parse config file: %s", filename);
    return(2);
  }

  // Open log file
  std::ifstream ifs (argv[1], std::ifstream::in);
  if (ifs.fail()) {
    std::cerr << "Failed to open file " << argv[1] << std::endl;
    return 2;
  }

  process_log_file(ifs, config);
}
