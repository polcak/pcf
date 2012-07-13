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
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <libxml/xmlreader.h>
#include <libxml/xmlwriter.h>
#include <string.h>

#include "check_computers.h"
#include "list.h"


#define MY_ENCODING "UTF-8"


/// Found computer name
xmlChar *computer_name = NULL;


/**
 * Make the main structure of the document
 * @param[in] filename Filename
 * @return 0 if ok
 */
int first_computer(const char *filename);

/**
 * Print node and info (depends on actual reader position)
 * Set name of the recognized computer
 * @param[in] reader Actual position in tree
 */
void print_node(xmlTextReaderPtr reader);


int first_computer(const char *filename)
{
  int rc;
  xmlTextWriterPtr writer;
  xmlDocPtr doc;
  
  writer = xmlNewTextWriterDoc(&doc, 0);
  if (writer == NULL)
    return(1);

  /// Default header
  rc = xmlTextWriterStartDocument(writer, NULL, MY_ENCODING, NULL);
  if (rc < 0)
    return(1);

  /// Root node <computers>
  rc = xmlTextWriterStartElement(writer, BAD_CAST "computers");
  if (rc < 0)
    return(1);
  
  /// </computers>
  rc = xmlTextWriterEndDocument(writer);
  if (rc < 0)
    return(1);

  /// Save
  xmlFreeTextWriter(writer);
  xmlSaveFileEnc(filename, doc, MY_ENCODING);
  xmlFreeDoc(doc);
  
  return(0);
}

int save_computer(const char *name, double skew, int freq, const char *address)
{
  xmlDocPtr doc;
  xmlNodePtr nodeptr = NULL, node = NULL, node_child = NULL;

  /// File doesn't exist yet
  if (access(database, F_OK) != 0) {
    if (first_computer(database) != 0) {
      fprintf(stderr, "Cannot create XML file: %s\n", database);
      return(1);
    }
  }
  
  doc = xmlParseFile(database);
  if (doc == NULL ) {
    fprintf(stderr, "XML document not parsed successfully: %s\n", database);
    return(1);
  }
  
  nodeptr = xmlDocGetRootElement(doc);
  if (nodeptr == NULL) {
    xmlFreeDoc(doc); 
    return(1);
  }
  
  /// Check root (<computers>)
  if (xmlStrcmp(nodeptr->name, (const xmlChar *) "computers")) {
    fprintf(stderr, "XML document of the wrong type: %s\n", database);
    xmlFreeDoc(doc);
    return(1);
  }
  
  /// <computer skew>
  node = xmlNewNode(NULL, BAD_CAST "computer");
  char tmp[30];
  sprintf(tmp, "%lf", skew);
  xmlNewProp(node, BAD_CAST "skew", BAD_CAST tmp);
  xmlAddChild(nodeptr , node);

  /// <name>
  node_child = xmlNewChild(node, NULL, BAD_CAST "name", BAD_CAST name);
  xmlAddChild(node, node_child);
  xmlAddChild(nodeptr, node);
  
  /// <address>
  node_child = xmlNewChild(node, NULL, BAD_CAST "address", BAD_CAST address);
  xmlAddChild(node, node_child);
  xmlAddChild(nodeptr, node);
  
  /// <freq>
  sprintf(tmp, "%d", freq);
  node_child = xmlNewChild(node, NULL, BAD_CAST "frequency", BAD_CAST tmp);
  xmlAddChild(node, node_child);
  xmlAddChild(nodeptr, node);
  
  /// <date>
  time_t rawtime;
  time(&rawtime);
  node_child = xmlNewChild(node, NULL, BAD_CAST "date", BAD_CAST ctime(&rawtime));
  xmlAddChild(node, node_child);
  xmlAddChild(nodeptr, node);

  /// Save
  xmlSaveFileEnc(database, doc, MY_ENCODING);
  xmlFreeDoc(doc);
  
  return(0);
}

void print_node(xmlTextReaderPtr reader)
{
  static short save = 0;
  xmlChar *name, *value;
  
  /// Name
  name = xmlTextReaderName(reader);
  if (xmlStrcmp(name, BAD_CAST "name") == 0) {
#ifdef DEBUG
    printf("Name: ");
#endif
    save = 1;
  }
#ifdef DEBUG
  else if (xmlStrcmp(name, BAD_CAST "address") == 0)
    printf("Address: ");
  else if (xmlStrcmp(name, BAD_CAST "frequency") == 0)
    printf("Frequency: ");
  else if (xmlStrcmp(name, BAD_CAST "date") == 0)
    printf("Date: ");
#endif
  xmlFree(name);
  
  /// Value
  if (xmlTextReaderHasValue(reader)) {
    value = xmlTextReaderValue(reader);
#ifdef DEBUG
    printf("%s\n", value);
#endif
    if (save) {
      computer_name = value;
      save = 0;
    }
    else
      xmlFree(value);
  }
}

char *check_computers(double skew, double *diff)
{
  /// No computers
  if (access(database, F_OK) != 0) {
    return(NULL);
  }
  
  xmlTextReaderPtr reader;
  reader = xmlNewTextReaderFilename(database);
  
  if (!reader)
    return(NULL);
  
  double skew2, tmp_skew, best_skew = 0.0;
  short print = 0;
  short nl = 0;
  short first = 1;
  while (xmlTextReaderRead(reader) == 1) {
    
    /// End of element
    if (xmlTextReaderNodeType(reader) == 15)
      continue;
    
    if (xmlTextReaderHasAttributes(reader)) {
      skew2 = atof((char *)xmlTextReaderGetAttribute(reader, BAD_CAST "skew"));
      
      /// Comparison
      tmp_skew = fabs(skew - skew2);
      if (tmp_skew < THRESHOLD) {
	if (first == 1 || (tmp_skew < best_skew)) {
#ifdef DEBUG
	  printf("\n\nSkew: %f (now: %f. diff: %f)\n", skew2, skew, tmp_skew);
#endif
	  best_skew = tmp_skew;
	  *diff = best_skew;
	  print = 1;
	  nl = 1;
	  first = 0;
	}
      }
      else if (print == 1)
	print = 0;
    }
    /// Print computer and set name
    if (print)
      print_node(reader);
  }
  
  if (nl) {
#ifdef DEBUG
    printf("\n");
#endif
  }
  else
    computer_name = NULL;
  
  xmlTextReaderClose(reader);
  
  return((char *) computer_name);
}

char *check_actives(computer_info *list, computer_info *current_list)
{
  computer_info *tmp;
  for (tmp = list; tmp != NULL; tmp = tmp->next_computer) {
    if (tmp == current_list)
      continue;
    if (tmp->freq == 0)
      continue;
    if (fabs(tmp->skew.alpha - current_list->skew.alpha) < THRESHOLD) {
      current_list->skew.diff = fabs(tmp->skew.alpha - current_list->skew.alpha);
      return(tmp->address);
    }
  }
  
  return(NULL);
}

int save_active(computer_info *list)
{
  xmlDocPtr doc;
  xmlNodePtr nodeptr = NULL, node = NULL, node_child = NULL;

  /// File doesn't exist yet
  if (first_computer(active) != 0) {
    fprintf(stderr, "Cannot create XML file: %s\n", active);
    return(1);
  }
    
  doc = xmlParseFile(active);
  if (doc == NULL ) {
    fprintf(stderr, "XML document not parsed successfully: %s\n", active);
    return(1);
  }
  
  nodeptr = xmlDocGetRootElement(doc);
  if (nodeptr == NULL) {
    xmlFreeDoc(doc); 
    return(1);
  }
  
  /// Check root (<computers>)
  if (xmlStrcmp(nodeptr->name, (const xmlChar *) "computers")) {
    fprintf(stderr, "XML document of the wrong type: %s\n", active);
    xmlFreeDoc(doc);
    return(1);
  }
  
  if (list == NULL)
    return(0);
  
  computer_info *current_list;
  
  for (current_list = list; current_list != NULL; current_list = current_list->next_computer) {
    
    if (current_list->freq == 0) {
#ifdef DEBUG
      fprintf(stderr, "XML: skipping %s - frequency is 0\n", current_list->address);
#endif
      continue;
    }
  
    /// <computer skew>
    node = xmlNewNode(NULL, BAD_CAST "computer");
    char tmp[30];
    sprintf(tmp, "%lf", current_list->skew.alpha);
    xmlNewProp(node, BAD_CAST "skew", BAD_CAST tmp);
    xmlAddChild(nodeptr , node);

    /// <name>
    if (current_list->name) {
      node_child = xmlNewChild(node, NULL, BAD_CAST "name", BAD_CAST current_list->name);
      xmlAddChild(node, node_child);
      xmlAddChild(nodeptr, node);
      
      /// <diff>
      sprintf(tmp, "%lf", current_list->skew.diff);
      node_child = xmlNewChild(node, NULL, BAD_CAST "diff", BAD_CAST tmp);
      xmlAddChild(node, node_child);
      xmlAddChild(nodeptr, node);
    }
    
    /// <address>
    node_child = xmlNewChild(node, NULL, BAD_CAST "address", BAD_CAST current_list->address);
    xmlAddChild(node, node_child);
    xmlAddChild(nodeptr, node);
    
    /// <freq>
    sprintf(tmp, "%d", current_list->freq);
    node_child = xmlNewChild(node, NULL, BAD_CAST "frequency", BAD_CAST tmp);
    xmlAddChild(node, node_child);
    xmlAddChild(nodeptr, node);
    
    /// <packets>
    sprintf(tmp, "%ld", current_list->count);
    node_child = xmlNewChild(node, NULL, BAD_CAST "packets", BAD_CAST tmp);
    xmlAddChild(node, node_child);
    xmlAddChild(nodeptr, node);
    
    /// <date>
    node_child = xmlNewChild(node, NULL, BAD_CAST "date", BAD_CAST ctime(&current_list->rawtime));
    xmlAddChild(node, node_child);
    xmlAddChild(nodeptr, node);
    
    /// Save
    xmlSaveFileEnc(active, doc, MY_ENCODING);

#ifdef DEBUG
    fprintf(stderr, "XML: saved %s: frequency %d, skew %lf\n", current_list->address, current_list->freq, current_list->skew.alpha);
#endif
  }
  
  xmlFreeDoc(doc);
  
  return(0);
}
