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

#ifndef _PARSE_CONFIG_H
#define _PARSE_CONFIG_H


/**
 * Structure with all config data
 */
typedef struct pcf_config pcf_config;

struct pcf_config {
  char dev[10];
  int number;
  unsigned int time;
  int port;
  char src[16];
  char dst[16];
  short syn;
  short ack;
  char filter[4096];
  
  char active[1024];
  char database[1024];
  
  int block;
  int time_limit;
  double threshold;
};

/**
 * Fill the config structure
 * @param[in] filename Config file name
 * @return Filled config structure, NULL if error
 */
pcf_config *get_config(const char *filename);

/**
 * Free the config structure
 * @param[in] config Structure with config data
 */
void free_config(pcf_config *config);


#endif
