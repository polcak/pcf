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
#include <string.h>
#include "parse_config.h"

#define MAXLEN 254


/**
 * Fill the config structure with default data
 * @param[in] config Config structure
 */
void init(pcf_config *config)
{
  strcpy(config->dev, "");
  config->number = 0;
  config->time = 0;
  config->port = 0;
  strcpy(config->src, "");
  strcpy(config->dst, "");
  config->syn = 0;
  config->ack = 0;
  strcpy(config->filter, "");
  
  strcpy(config->active, "www/data/active.xml");
  strcpy(config->database, "www/data/database.xml");
  
  config->block = 100;
  config->time_limit = 3600;
  config->threshold = 0.001;
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

pcf_config *get_config(const char *filename)
{
  FILE *f;
  f = fopen(filename, "r");
  if (f == NULL) {
    fprintf(stderr, "Cannot open config file: %s\n", filename);
    return NULL;
  }
  
  /// Initialisation
  pcf_config *config = (pcf_config*)malloc(sizeof(pcf_config));
  init(config);
  
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
        strncpy(config->dev, value, strlen(value));
      }
      // num_packets
      else if (strcmp(name, "num_packets") == 0) {
        config->number = atoi(value);
      }
      // time
      else if (strcmp(name, "time") == 0) {
        config->time = atoi(value);
      }
      // port
      else if (strcmp(name, "port") == 0) {
        config->port = atoi(value);
        if (config->port < 0 || config->port > 65535) {
          fprintf(stderr, "Config: Wrong port number\n");
          config->port = 0;
        }
      }
      // source_address
      else if (strcmp(name, "src_address") == 0) {
        if (strlen(value) > 16) {
          fprintf(stderr, "Config: Source address too long (%s)\n", value);
          break;
        }
        strncpy(config->src, value, strlen(value));
      }
      // destination address
      else if (strcmp(name, "dst_address") == 0) {
        if (strlen(value) > 16) {
          fprintf(stderr, "Config: Destination address too long (%s)\n", value);
          break;
        }
        strncpy(config->dst, value, strlen(value));
      }
      // syn
      else if (strcmp(name, "syn") == 0)
        config->syn = atoi(value);
      // ack
      else if (strcmp(name, "ack") == 0)
        config->ack = atoi(value);
      // filter
      else if (strcmp(name, "filter") == 0)
        strncpy(config->filter, value, strlen(value));
      
      // active
      else if (strcmp(name, "active") == 0)
        strncpy(config->active, value, strlen(value));
      
      // database
      else if (strcmp(name, "database") == 0)
        strncpy(config->database, value, strlen(value));
      
      // BLOCK
      else if (strcmp(name, "BLOCK") == 0) {
        config->block = atoi(value);
        if (config->block == 0)
          config->block = 100;
      }
      // TIME_LIMIT
      else if (strcmp(name, "TIME_LIMIT") == 0) {
        config->time_limit = atoi(value);
        if (config->time_limit == 0)
          config->time_limit = 3600;
      }
      // THRESHOLD
      else if (strcmp(name, "THRESHOLD") == 0) {
        config->threshold = atof(value);
        /*if (config->threshold == 0.0)
          config->threshold = 0.001;*/
      }
    }
  }
  
  if (fclose(f) != 0)
    fprintf(stderr, "Cannot close config file: %s\n", filename);
  
  return config;
}

void free_config(pcf_config *config)
{
  if (config != NULL)
    free(config);
}
