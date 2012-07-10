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
#include <math.h>
#include <time.h>

#include "list.h"
#include "computations.h"
#include "check_computers.h"


/// List of all packet lists
my_list *list = NULL;


/**
 * Insert new packet into the list
 * @param[in] tail Pointer to the end of the list
 * @param[in] time Arrival time of the new packet
 * @param[in] timestamp Timestamp of the new packet
 * @return 0 if ok
 */
my_packet *insert_packet(my_packet *tail, double time, unsigned long int timestamp);

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
my_packet *remove_packet(my_packet *current);

/**
 * Reduce list of packets - only ascending/descending packets stay
 * @param[in] list Pointer to the list of packets
 */
void reduce_packets(my_list *list);

/** 
 * Save 'count' packets into file (called 'IP address.log')
 * Compute and set frequency before saving
 * @param[in] current_list Pointer to header of packet list
 * @param[in] count Number of packets to save
 * @return 0 if ok
 * */
int save_packets(my_list* current_list, int count, short int first);

/**
 * Print uptime
 * @param[in] tail Pointer to the last packet of the list
 * @param[in] freq Frequency
 */
void print_uptime(my_packet *tail, const int freq);

/**
 * Generate graph
 * @param[in] list Pointer to header of packet list
 */
void generate_graph(my_list *current_list);


int new_packet(const char *address, double ttime, unsigned long int timestamp)
{
  static unsigned long total = 0;
  printf("\r%lu packets captured", ++total);
  fflush(stdout);
  
  if (list != NULL) {
    my_list *current_list;
    for (current_list = list; current_list != NULL; current_list = current_list->next) {
      if (strcmp(current_list->address, address) == 0) {
	
	/// Too much time since last packet so start from the beginning
	if ((ttime - current_list->tail->time) > TIME_LIMIT) {
	  remove_old_lists(ttime);
	  return(3);
	}
	
	/// Check if packet has the same or lower timestamp
	if (timestamp <= current_list->tail->timestamp) {
#ifdef DEBUG
	  if (timestamp < current_list->tail->timestamp)
	    fprintf(stderr, "%s: Lower timestamp\n", current_list->address);
#endif
	  return(1);
	}
	
	/// Skip packets in first minute
	if ((ttime - current_list->head->time) < 60)
	  return(2);
	
	/// Stop supporting lists with stupid frequency
	if (fabs(current_list->freq) > 10000)
	  return(0);
	
	/// Insert packet
	current_list->tail = insert_packet(current_list->tail, ttime, timestamp);
	if (current_list->tail == NULL)
	  return(-1);
	
	/// Increment number of packets
	current_list->count++;
  
	/// Operations do every BLOCK
	if ((current_list->count % BLOCK) == 0) {
	  
	  /// Remove old lists
	  remove_old_lists(ttime);
	  
	  /// Set frequency
	  if (current_list->freq == 0)
	    current_list->freq = get_frequency(current_list->head);
	  
	  /// Set offsets
	  set_offsets(current_list->head, current_list->head, current_list->freq);
	  
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
	  save_packets(current_list, packets_count(current_list->head), current_list->first);
	  
	  /// Set skew
	  if (set_skew(current_list) != 0)
	    return(1);
	  
	  /// Ignore list if skew is smaller than THRESHOLD
	  if (fabs(current_list->skew.alpha) < THRESHOLD)
	    return(0);
	  
	  /// Check active computers
	  char *tmp = check_actives(list, current_list);
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
	  save_active(list);
	  
	  /// Too much packets -> remove packets from the beginning
	  while (current_list->count > (BLOCK * 100)) {
	    for (int i = 0; i < BLOCK; i++) {
	      current_list->head = remove_packet(current_list->head);
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
  my_list *new_list = (my_list*)malloc(sizeof(my_list));
  if (!new_list) {
    fprintf(stderr, "Malloc: Not enough memory\n");
    return(-1);
  }
  
  /// Set address
  strcpy(new_list->address, address);
  
  new_list->freq = 0;
  new_list->skew.alpha = 0;
  new_list->tail = NULL;
  new_list->name = NULL;
  time(&new_list->rawtime);
  
  new_list->first = 1;
  
  new_list->tail = insert_packet(new_list->tail, ttime, timestamp);
  if (new_list->head == NULL)
    return(-1);
  new_list->count = 1;
  
  new_list->head = new_list->tail;
  
  new_list->next = list;
  list = new_list;
  
  return(0);
}

my_packet *insert_packet(my_packet *tail, double time, unsigned long int timestamp)
{  
  my_packet *new_packet = (my_packet*)malloc(sizeof(my_packet));
  if (!new_packet) {
    fprintf(stderr, "Malloc: Not enough memory\n");
    return(NULL);
  }
  
  new_packet->time = time;
  new_packet->timestamp = timestamp;
  
  if (tail == NULL) {
    tail = new_packet;
    tail->prev = NULL;
    tail->next = NULL;
  }
  else {
    tail->next = new_packet;
    tail->next->prev = tail;
    tail->next->next = NULL;
  }
  
#ifdef PACKETS
  printf("Time: %.6lf\n", new_packet->time);
  printf("Timestamp: %lu\n\n", new_packet->timestamp);
#endif
  
  return(new_packet);
}

void remove_old_lists(double time)
{
  if (list == NULL)
    return;
  
  my_list *current_list;
  my_list *tmp = list;
  
  /// Removing first list
  while ((time - list->tail->time) > TIME_LIMIT) {
    list = list->next;
    if (tmp->name != NULL)
      free(tmp->name);
    free(tmp);
    tmp = list;
    if (list == NULL)
      return;
  }
  
  for (current_list = list->next; current_list != NULL; current_list = current_list->next) {
    if ((time - current_list->tail->time) > TIME_LIMIT) {
      tmp->next = current_list->next;
      if (current_list->name != NULL)
	free(current_list->name);
      free(current_list);
      current_list = tmp;
    }
    else
      tmp = current_list;
  }
}

my_packet *remove_packet(my_packet *current)
{
  if (current == NULL)
    return(NULL);
  
  if (current->prev != NULL)
    current->prev->next = current->next;
  if (current->next != NULL)
    current->next->prev = current->prev;
  
  my_packet *tmp;
  
  if (current->prev != NULL)
    tmp = current->prev;
  else
    tmp = current->next;
  
  free(current);
  
  return(tmp);
}

void reduce_packets(my_list *current_list)
{ 
  if (current_list->count < 4)
    return;
  
  if (current_list->skew.alpha == 0)
    return;
  
  my_packet *current;
  
  /// Don't reduce if skew is too small
  if (current_list->skew.alpha < 0.01 && current_list->skew.alpha > -0.01)
    return;
  
  /// Ascending
  if (current_list->skew.alpha > 0) {
    current = current_list->head;
    while (current_list->count > 3 && current->next != NULL) {
      if (current->next->offset.y <= current->offset.y) {
	current = remove_packet(current->next);
	current_list->count--;
      }
      else
	current = current->next;
    }
    current_list->tail = current;
  }
  
  /// Descending
  else {
    current = current_list->tail;
    while (current_list->count > 3 && current->prev != NULL) {
      if (current->prev->offset.y <= current->offset.y) {
	current = remove_packet(current->prev);
	current = current->next;
	current_list->count--;
      }
      else
	current = current->prev;
    }
    current_list->head = current;
  }
}

int save_packets(my_list *current_list, int count, short first)
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
  
  /// Open file
  if (first == 1)
    f = fopen(filename, "w");
  else
    f = fopen(filename, "a");
  
  if (f == NULL) {
    fprintf(stderr, "Cannot save packets into the file: %s\n", filename);
    return(2);
  }
  
  /// Get back
  my_packet *current = current_list->tail;
  while (count-- > 1 && current != NULL)
    current = current->prev;
  
  /// Write to file
  char str[100];
  for (; current != NULL; current = current->next) {
    sprintf(str, "%lf\t%lf\n", current->offset.x, current->offset.y);
    fputs(str, f);
  }
  
  /// Close file
  if (fclose(f) != 0)
    fprintf(stderr, "Cannot close file: %s\n", filename);
  
  return(0);
}

unsigned long packets_count(my_packet *head)
{
  my_packet *current;
  unsigned long result = 0;
  
  for (current = head; current != NULL; current = current->next)
    result++;
  
  return(result);
}

void process_results(short save, short uptime, short graph)
{
  printf("-----------\n"
	 "- Results -\n"
	 "-----------\n\n");
  
  if (list != NULL) {
    my_list *current_list;
    for (current_list = list; current_list != NULL; current_list = current_list->next) {
      
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
	print_uptime(current_list->tail, current_list->freq);
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

void print_uptime(my_packet *tail, int freq)
{
  time_t now = time(NULL);
  int uptime = (int)round((tail->timestamp / freq));
  
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

void generate_graph(my_list *current_list)
{
  FILE *f;
  char filename[] = "graph/graph.gp";
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
  
  system("gnuplot graph/graph.gp");
  
  return;
}

void free_memory()
{
  if (list != NULL) {
    my_list *current_list = list;
    my_packet *current;
    
    while (current_list != NULL) {
      current = current_list->head;
      while (current != NULL) {
	current_list->head = current->next;
	free(current);
	current = current_list->head;
      }
      list = current_list->next;
      if (current_list->name != NULL)
	free(current_list->name);
      free(current_list);
      current_list = list;
    }
  }
}
