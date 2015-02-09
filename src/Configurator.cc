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
#include <iostream>
#include <string.h>
#include "Configurator.h"

#define MAXLEN 4096


Configurator * Configurator::innerInstance = NULL;
const std::string Configurator::xmlDir  = "www/data/";


Configurator * Configurator::instance() {
    if(innerInstance == NULL) {
        innerInstance = new Configurator();
    }
    
    return innerInstance;
}

/**
 * Fill the config structure with default data
 * @param[in] config Config structure
 */
void Configurator::Init()
{
  icmpDisable = false;
  javacriptDisable = false;
  portEnable = false;
  datalink = "";
  datafile = "";
  dev[0] = 0;
#ifdef DEBUG
  verbose = true;
#else
  verbose = false;
#endif
  exportSkewChanges = false;
  number = 0;
  time = 0;
  port = 0;
  strcpy(src, "");
  strcpy(dst, "");
  syn = 0;
  ack = 0;
  strcpy(filter, "");
  
  strcpy(active, "active.xml");
  strcpy(database, "database.xml");
  
  block = 100;
  timeLimit = 3600;
  threshold = 0.001;
  reduce = false;
  xmlRefreshLimit = 60;
}

/**
 * Remove whitespaces from the beginning and from the end of every record
 * @param[in] buf String with record
 */
void remove_whitespaces(char *buf)
{
  /// Remove whitespaces from the beginning
  size_t i = 0;
  while (buf[i] == ' ' || buf[i] == '\t' || buf[i] == '\r' || buf[i] == '\n')
    i++;
  
  /// Move the string
  if (i > 0) {
    size_t j;
    for (j = 0; j < strlen(buf); j++)
      buf[j] = buf[i + j];
    buf[j] = '\0';
  }
  
  /// Remove whitespaces from the end
  i = strlen(buf) - 1;
  while (buf[i] == ' ' || buf[i] == '\t' || buf[i] == '\r' || buf[i] == '\n')
    i--;
  if (i < strlen(buf) - 1)
    buf[i + 1] = '\0';
}

/**
 * Parse the record
 * @param[in] buf String with record (i.e. 'port 80')
 * @param[out] name Name (i.e. 'port')
 * @param[out] value Value (i.e. '80')
 * @return 0 if ok
 */
int parse(const char *buf, char *name, char *value)
{  
  /// Empty or commentary line
  if (strlen(buf) < 1 || buf[0] == '#')
    return(2);
  
  int i = 0;
  
  /// Name
  while (buf[i] != ' ' && buf[i] != '\t')
    i++;
  strncpy(name, buf, i);
  name[i] = '\0';
  
  while (buf[i] == ' ' && buf[i] == '\t')
    i++;
  
  /// Value
  strncpy(value, buf + i + 1, strlen(buf) - i);
  value[strlen(buf) - i] = '\0';
  
  return(0);
}

void Configurator::GetConfig(const char *filename)
{
  FILE *f;
  f = fopen(filename, "r");
  if (f == NULL) {
    fprintf(stderr, "Cannot open config file: %s\n", filename);
    exit(2);
  }
  
  /// Initialization
  Init();
  
  /// Parse the file
  char buf[MAXLEN];
  char name[MAXLEN];
  char value[MAXLEN];
  while (!feof(f)) {
    if (fgets(buf, MAXLEN - 2, f) == NULL)
      break;
    remove_whitespaces(buf);
    if (parse(buf, name, value) == 0) {
      // interface
      if (strcmp(name, "interface") == 0) {
        if (strlen(value) > 10) {
          fprintf(stderr, "Config: Device name too long (%s)\nSetting to any", value);
          break;
        }
        strncpy(dev, value, strlen(value));
      }
      // num_packets
      else if (strcmp(name, "num_packets") == 0) {
        number = atoi(value);
      }
      // time
      else if (strcmp(name, "time") == 0) {
        time = atoi(value);
      }
      // port
      else if (strcmp(name, "port") == 0) {
        port = atoi(value);
        if (port < 0 || port > 65535) {
          fprintf(stderr, "Config: Wrong port number\n");
          port = 0;
        }
      }
      // source_address
      else if (strcmp(name, "src_address") == 0) {
        if (strlen(value) > 16) {
          fprintf(stderr, "Config: Source address too long (%s)\n", value);
          break;
        }
        strncpy(src, value, strlen(value));
      }
      // destination address
      else if (strcmp(name, "dst_address") == 0) {
        if (strlen(value) > 16) {
          fprintf(stderr, "Config: Destination address too long (%s)\n", value);
          break;
        }
        strncpy(dst, value, strlen(value));
      }
      // syn
      else if (strcmp(name, "syn") == 0)
        syn = atoi(value);
      // ack
      else if (strcmp(name, "ack") == 0)
        ack = atoi(value);
      // filter
      else if (strcmp(name, "filter") == 0)
        strncpy(filter, value, strlen(value));
      
      // active
      else if (strcmp(name, "active") == 0)
        strncpy(active, value, strlen(value));
      
      // database
      else if (strcmp(name, "database") == 0)
        strncpy(database, value, strlen(value));
      
      // BLOCK
      else if (strcmp(name, "BLOCK") == 0) {
        block = atoi(value);
        if (block == 0)
          block = 100;
      }
      // TIME_LIMIT
      else if (strcmp(name, "TIME_LIMIT") == 0) {
        timeLimit = atoi(value);
        if (timeLimit == 0)
          timeLimit = 3600;
      }
      // THRESHOLD
      else if (strcmp(name, "THRESHOLD") == 0) {
        threshold = atof(value);
        /*if (threshold == 0.0)
          threshold = 0.001;*/
      }
      // REFRESH_TIME_LIMIT
      else if (strcmp(name, "REFRESH_TIME_LIMIT") == 0) {
        xmlRefreshLimit = atof(value);
        if (xmlRefreshLimit == 0)
          xmlRefreshLimit = 60;
      }
    }
  }
  
  if (fclose(f) != 0)
    fprintf(stderr, "Cannot close config file: %s\n", filename);
  
  return;
}
