/**
 * Copyright (C) 2012 Jakub Jirasek <xjiras02@stud.fit.vutbr.cz>
 *                    Libor Polčák <ipolcak@fit.vutbr.cz>
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
#include <math.h>
#include <time.h>

#include "list.h"
#include "computations.h"
#include "check_computers.h"


/// List of all packet lists
computer_info *all_known_computers = NULL;


/**
 * Insert new packet into the list
 * @param[in] tail_packet Pointer to the end of the list
 * @param[in] time Arrival time of the new packet
 * @param[in] timestamp Timestamp of the new packet
 * @return 0 if ok
 */
packet_time_info *insert_packet(packet_time_info *tail_packet, double time, unsigned long int timestamp);

/**
 * Remove lists older (last recieved packet) than TIME_LIMIT
 * @param[in] time Time of the last packet
 */
void remove_old_lists(double time);

/**
 * Remove packet from the list
 * @param[in] current Pointer to the packet to be removed
 * @return Packet next to the removed packet
 */
packet_time_info *remove_packet(packet_time_info *current);

/**
 * Reduce list of packets - only ascending/descending packets stay
 * @param[in] list Pointer to the list of packets
 */
void reduce_packets(computer_info *list);

/** 
 * Save 'count' packets into file (called 'IP address.log')
 * Compute and set frequency before saving
 * @param[in] current_list Pointer to header of packet list
 * @param[in] count Number of packets to save
 * @param[in] rewrite Boolean that conrols if the file is overwritten or the packets are appended
 * @return 0 if ok
 * */
int save_packets(computer_info* current_list, int count, short int rewrite);

/**
 * Print uptime
 * @param[in] tail_packet Pointer to the last packet of the list
 * @param[in] freq Frequency
 */
void print_uptime(packet_time_info *tail_packet, const int freq);

/**
 * Generate graph
 * @param[in] list Pointer to header of packet list
 */
void generate_graph(computer_info *current_list);


int new_packet(const char *address, double ttime, unsigned long int timestamp)
{
  static unsigned long total = 0;
  printf("\r%lu packets captured", ++total);
  fflush(stdout);

  if (all_known_computers != NULL) {
    computer_info *current_list;
    for (current_list = all_known_computers; current_list != NULL; current_list = current_list->next_computer) {
      if (strcmp(current_list->address, address) == 0) {

        /// Too much time since last packet so start from the beginning
        if ((ttime - current_list->tail_packet->time) > TIME_LIMIT) {
          remove_old_lists(ttime);
          return(3);
        }

        /// Check if packet has the same or lower timestamp
        if (timestamp <= current_list->tail_packet->timestamp) {
#ifdef DEBUG
          if (timestamp < current_list->tail_packet->timestamp)
            fprintf(stderr, "%s: Lower timestamp\n", current_list->address);
#endif
          return(1);
        }

        /// Stop supporting lists with stupid frequency
        if (fabs(current_list->freq) > 10000)
          return(0);

        /// Insert packet
        current_list->tail_packet = insert_packet(current_list->tail_packet, ttime, timestamp);
        if (current_list->tail_packet == NULL)
          return(-1);

        /// Increment number of packets
        current_list->count++;
#if 0
        fprintf(stderr, "%s: %ld\n", current_list->address, current_list->count);
#endif

        /// Operations do every BLOCK
        if ((current_list->count % BLOCK) == 0) {

          /// Remove old lists
          remove_old_lists(ttime);

          /// Set frequency
          if (current_list->freq == 0) {
            if ((ttime - current_list->head_packet->time) < 60) {
              return 2;
            }
            else {
              current_list->freq = compute_clock_frequency(current_list->head_packet);
#if 0
              fprintf(stderr, "Found %s with frequency %d", current_list->address, current_list->freq);
#endif
            }
          }

          /// Set offsets
          set_offsets(current_list->head_packet, current_list->head_packet, current_list->freq);

          /// Set date
          time(&current_list->rawtime);

          /// Save offsets into file - all
          //save_packets(current_list, BLOCK, current_list->first);
          //current_list->first = 0;

          /// Reduce packets
          static short reduce = 0;
          if (reduce || current_list->count > (BLOCK * 5)) {
            reduce_packets(current_list);
            reduce = 1;
          }

          /// Save offsets into file - reduced
          save_packets(current_list, packets_count(current_list->head_packet), 1);

          /// Set skew
          if (set_skew(current_list) != 0) {
#ifdef DEBUG
            fprintf(stderr, "Clock skew not set for %s\n", current_list->address);
#endif
            return(1);
          }

          /// Check active computers
          char *tmp = check_actives(all_known_computers, current_list);
          if (tmp)
            current_list->name = tmp;

          /// Check saved computers
          else {
            char *tmp = check_computers(current_list->skew.alpha, &current_list->skew.diff);
            if (tmp)
              current_list->name = tmp;
            else
              current_list->name = NULL;
          }

          /// Save active computers
          save_active(all_known_computers);

          /// Too much packets -> remove packets from the beginning
          while (current_list->count > (BLOCK * 100)) {
            for (int i = 0; i < BLOCK; i++) {
              current_list->head_packet = remove_packet(current_list->head_packet);
              current_list->count--;
            }
          }

          /// Generate graph
          generate_graph(current_list);

          /// WWW
          system("./gen_pics.sh 1>/dev/null");
        }

        return(0);
      }
    }
  }
  
  /// New packet list
  computer_info *new_list = (computer_info*)malloc(sizeof(computer_info));
  if (!new_list) {
    fprintf(stderr, "Malloc: Not enough memory\n");
    return(-1);
  }
  
  /// Set address
  strcpy(new_list->address, address);
  
  new_list->freq = 0;
  new_list->skew.alpha = 0;
  new_list->tail_packet = NULL;
  new_list->name = NULL;
  time(&new_list->rawtime);
  
  new_list->tail_packet = insert_packet(new_list->tail_packet, ttime, timestamp);
  if (new_list->head_packet == NULL)
    return(-1);
  new_list->count = 1;
  
  new_list->head_packet = new_list->tail_packet;
  
  // Insert new computer to the beginning of the list of all known computers
  new_list->next_computer = all_known_computers;
  all_known_computers = new_list;
  
  return(0);
}

packet_time_info *insert_packet(packet_time_info *tail_packet, double time, unsigned long int timestamp)
{  
  packet_time_info *new_packet = (packet_time_info*)malloc(sizeof(packet_time_info));
  if (!new_packet) {
    fprintf(stderr, "Malloc: Not enough memory\n");
    return(NULL);
  }
  
  new_packet->time = time;
  new_packet->timestamp = timestamp;
  
  if (tail_packet == NULL) {
    tail_packet = new_packet;
    tail_packet->prev_packet = NULL;
    tail_packet->next_packet = NULL;
  }
  else {
    tail_packet->next_packet = new_packet;
    tail_packet->next_packet->prev_packet = tail_packet;
    tail_packet->next_packet->next_packet = NULL;
  }
  
#ifdef PACKETS
  printf("Time: %.6lf\n", new_packet->time);
  printf("Timestamp: %lu\n\n", new_packet->timestamp);
#endif
  
  return(new_packet);
}

void remove_old_lists(double time)
{
  if (all_known_computers == NULL)
    return;
  
  computer_info *current_list;
  computer_info *tmp = all_known_computers;
  
  /// Removing first list
  while ((time - all_known_computers->tail_packet->time) > TIME_LIMIT) {
    all_known_computers = all_known_computers->next_computer;
    if (tmp->name != NULL)
      free(tmp->name);
    free(tmp);
    tmp = all_known_computers;
    if (all_known_computers == NULL)
      return;
  }
  
  for (current_list = all_known_computers->next_computer; current_list != NULL; current_list = current_list->next_computer) {
    if ((time - current_list->tail_packet->time) > TIME_LIMIT) {
      tmp->next_computer = current_list->next_computer;
      if (current_list->name != NULL)
        free(current_list->name);
      free(current_list);
      current_list = tmp;
    }
    else
      tmp = current_list;
  }
}

packet_time_info *remove_packet(packet_time_info *current)
{
  if (current == NULL)
    return(NULL);
  
  if (current->prev_packet != NULL)
    current->prev_packet->next_packet = current->next_packet;
  if (current->next_packet != NULL)
    current->next_packet->prev_packet = current->prev_packet;
  
  packet_time_info *tmp;
  
  if (current->prev_packet != NULL)
    tmp = current->prev_packet;
  else
    tmp = current->next_packet;
  
  free(current);
  
  return(tmp);
}

void reduce_packets(computer_info *current_list)
{ 
  if (current_list->count < 4)
    return;
  
  if (current_list->skew.alpha == 0)
    return;
  
  packet_time_info *current;
  
  /// Don't reduce if skew is too small
  if (current_list->skew.alpha < 0.01 && current_list->skew.alpha > -0.01)
    return;
  
  /// Ascending
  if (current_list->skew.alpha > 0) {
    current = current_list->head_packet;
    while (current_list->count > 3 && current->next_packet != NULL) {
      if (current->next_packet->offset.y <= current->offset.y) {
        current = remove_packet(current->next_packet);
        current_list->count--;
      }
      else
        current = current->next_packet;
    }
    current_list->tail_packet = current;
  }
  
  /// Descending
  else {
    current = current_list->tail_packet;
    while (current_list->count > 3 && current->prev_packet != NULL) {
      if (current->prev_packet->offset.y <= current->offset.y) {
        current = remove_packet(current->prev_packet);
        current = current->next_packet;
        current_list->count--;
      }
      else
        current = current->prev_packet;
    }
    current_list->head_packet = current;
  }
}

int save_packets(computer_info *current_list, int count, short rewrite)
{
  /*
  if (count == 0) {
    count = (packets_count() % BLOCK);
    if (count == 0)
      return(0);
  }
  */
  
  FILE *f;
  char filename[50] = "log/";
  strcat(filename, current_list->address);
  strcat(filename, ".log");
#ifdef DEBUG
  int lines = 0;
#endif
  
  /// Open file
  if (rewrite == 1)
    f = fopen(filename, "w");
  else
    f = fopen(filename, "a");
  
  if (f == NULL) {
    fprintf(stderr, "Cannot save packets into the file: %s\n", filename);
    return(2);
  }
  
  /// Get back
  packet_time_info *current = current_list->tail_packet;
  while (count-- > 1 && current != NULL)
    current = current->prev_packet;
  
  /// Write to file
  char str[100];
  for (; current != NULL; current = current->next_packet) {
    sprintf(str, "%lf\t%lf\n", current->offset.x, current->offset.y);
    fputs(str, f);
  }
  
  /// Close file
  if (fclose(f) != 0)
    fprintf(stderr, "Cannot close file: %s\n", filename);
  
  return(0);
}

unsigned long packets_count(packet_time_info *head_packet)
{
  packet_time_info *current;
  unsigned long result = 0;
  
  for (current = head_packet; current != NULL; current = current->next_packet)
    result++;
  
  return(result);
}

void process_results(short save, short uptime, short graph)
{
  printf("-----------\n"
         "- Results -\n"
         "-----------\n\n");
  
  if (all_known_computers != NULL) {
    computer_info *current_list;
    for (current_list = all_known_computers; current_list != NULL; current_list = current_list->next_computer) {
      
      /// Freq == 0 => not enough packets
      if (current_list->freq == 0)
        continue;
      
      /// Stupid frequency
      if (fabs(current_list->freq) > 10000)
        continue;
      
      printf("%s (%ld packets)\n\n"
             "Frequency: %d\n", current_list->address, current_list->count, current_list->freq);
      /// Print uptime
      if (uptime)
        print_uptime(current_list->tail_packet, current_list->freq);
      printf("Skew: f(x) = %lfx + %lf\n\n", current_list->skew.alpha, current_list->skew.beta);
      
      /// Save new computer
      if (save) {
        printf("Save this computer? (y/n) [n]: ");
        if (getchar() == 'y') {
          printf("Name: ");
          
          /// Flush stdin
          int c;
          while ((c = fgetc(stdin)) != EOF && c != '\n' );
          
          /// Name
          char name[256];
          fgets(name, sizeof(name), stdin);
          name[255] = '\0';
          c = 0;
          while (name[c] != EOF && name[c] != '\n')
            c++;
          if (name[c] == '\n')
            name[c] = '\0';
          
          /// Save
          if (save_computer(name, current_list->skew.alpha, current_list->freq, current_list->address) != 0)
            fprintf(stderr, "Cannot save new computer\n");
        }
      }
      
      /// Generate graph
      if (graph)
        generate_graph(current_list);
    }
  }
}

void print_uptime(packet_time_info *tail_packet, int freq)
{
  time_t now = time(NULL);
  int uptime = (int)round((tail_packet->timestamp / freq));
  
  time_t tmp = difftime(now, uptime);
  char *date_uptime = ctime(&tmp);
  // Delete new line from the end of the string
  date_uptime[strlen(date_uptime)-1] = '\0';
  
  int seconds = uptime;
  uptime /= 60;
  int minutes = uptime % 60;
  uptime -= minutes;
  int hours = ((uptime / 60) % 24);
  uptime -= hours;
  int days = ((uptime / (60 * 24)));
  
  printf("Uptime: %dd %dh %dm (since %s, %d seconds)\n", days, hours, minutes, date_uptime, seconds);
}

void generate_graph(computer_info *current_list)
{
  static const char* filename_template =  "graph/%s.gp";
  FILE *f;
  int filename_max = strlen(filename_template) + ADDRESS_SIZE;
  char filename[filename_max + 1];
  snprintf(filename, filename_max, filename_template, current_list->address);
  f = fopen(filename, "w");
  
  if (f == NULL) {
    fprintf(stderr, "Cannot create file: %s\n", filename);
    return;
  }
  
  if (current_list->skew.alpha == 0.0)
    if (set_skew(current_list) != 0)
      return;
  
  char tmp[100];

  fputs("set encoding iso_8859_2\n"
        "set terminal postscript font \"DejaVuSans\"\n"
        //"set terminal postscript\n"
        "set output 'graph/", f);
        fputs(current_list->address, f);
        fputs(".ps'\n\n"
        //"set xlabel 'Elapsed time [s]'\n"
        "set xlabel \"\310as od po\350\341tku m\354\370en\355 [s]\"\n"
        //"set ylabel 'Offset [ms]'\n\n"
        "set ylabel \"Odchylka [ms]\"\n\n"
        "f(x) = ", f);
  
  /// f(x)
  sprintf(tmp, "%lf", current_list->skew.alpha);
  fputs(tmp, f);
  fputs("*x + ", f);
  sprintf(tmp, "%lf", current_list->skew.beta);
  fputs(tmp, f);
  fputs("\n\nset grid\n"
        "set title \"", f);
  
  /// Title
  time_t rawtime;
  time(&rawtime);
  sprintf(tmp, "%s", ctime(&rawtime));
  tmp[strlen(tmp)-1] = '\0';
  fputs(tmp, f);
  fputs("\\n", f);
  fputs(current_list->address, f);
  
  /// Recognized computer
  if (current_list->name != NULL) {
    fputs("\\n", f);
    fputs(current_list->name, f);
    fputs("\" textcolor lt 2", f);
  }
  else
    fputs("\\nunknown\" textcolor lt 1", f);
  
  /// Plot
  fputs("\n\n"
        "plot '", f);
  fputs("log/", f);
  fputs(current_list->address, f);
  fputs(".log' title '', f(x)", f);
  
  /// Legend
  fputs(" title 'f(x) = ", f);
  sprintf(tmp, "%lf", current_list->skew.alpha);
  fputs(tmp, f);
  fputs("*x + ", f);
  sprintf(tmp, "%lf", current_list->skew.beta);
  fputs(tmp, f);
  fputs("'", f);
  
  if (fclose(f) != 0)
    fprintf(stderr, "Cannot close file: %s\n", filename);
  
  static const char* gnuplot_template =  "gnuplot graph/%s.gp";
  int gnuplot_max = strlen(gnuplot_template) + ADDRESS_SIZE;
  char gnuplot_cmd[gnuplot_max + 1];
  snprintf(gnuplot_cmd, gnuplot_max, gnuplot_template, current_list->address);
  system(gnuplot_cmd);
  
  return;
}

void free_memory()
{
  if (all_known_computers != NULL) {
    computer_info *current_list = all_known_computers;
    packet_time_info *current;
    
    while (current_list != NULL) {
      current = current_list->head_packet;
      while (current != NULL) {
        current_list->head_packet = current->next_packet;
        free(current);
        current = current_list->head_packet;
      }
      all_known_computers = current_list->next_computer;
      if (current_list->name != NULL)
        free(current_list->name);
      free(current_list);
      current_list = all_known_computers;
    }
  }
}
