/**
* Copyright (C) 2012 Jakub Jirasek <xjiras02@stud.fit.vutbr.cz>
* Libor Polčák <ipolcak@fit.vutbr.cz>
* Barbora Frankova <xfrank08@stud.fit.vutbr.cz>
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
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with pcf. If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <time.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlwriter.h>
#include <stdbool.h>
#include <string.h>
#include <iomanip>

#include "check_computers.h"
#include "ComputerInfo.h"
#include "TimeSegment.h"
#include "Configurator.h"


#define MY_ENCODING "UTF-8"

bool find_computer_in_saved(double referenced_skew, identity_container &identities, const double THRESHOLD, const char *database)
{
  /// No computers
  if (access(database, F_OK) != 0) {
    return false;
  }
  //std::cout << "-------find comp in saved--------" << std::endl;
  xmlDoc *xml_doc = xmlReadFile(database, NULL, 0);

  if (xml_doc == NULL) {
    return(false);
  }

  xmlNode *root_element = xmlDocGetRootElement(xml_doc);

  for(xmlNode *computer_node = root_element->children; computer_node != NULL; computer_node = computer_node->next) {

    if (computer_node->type != XML_ELEMENT_NODE) {
      continue;
    }

    if (strcmp((char *) computer_node->name, "computer") != 0) {
      continue;
    }

    xmlChar *xml_skew_prop = xmlGetProp(computer_node, BAD_CAST "skew");
    if (xml_skew_prop != NULL) {
      double skew_xml = atof((char *) xml_skew_prop);

      /// Comparison
      double skew_diff = fabs(referenced_skew - skew_xml);
      if (skew_diff < THRESHOLD) {
        bool name_reached = false;
        xmlNode *computer_child_node = computer_node->children;

        while (!name_reached && computer_child_node != NULL) {
          if (computer_child_node->type != XML_ELEMENT_NODE) {
            continue;
          }

          if (strcmp((char *) computer_child_node->name, "name") == 0) {
            xmlChar *computer_name = xmlNodeGetContent(computer_child_node);
            if (computer_name != NULL) {
              identities.insert((char *) computer_name);
              name_reached = true;
              xmlFree(computer_name);
            }
          }
        }
      }
      xmlFree(xml_skew_prop);
    }
  }

  xmlFreeDoc(xml_doc);

  return true;
}

int save_active(const std::list<ComputerInfo *> &all_computers, const char *file, ComputerInfoList &computers)
{
  // check if time limit passed
  time_t currentTime;
  time(&currentTime);
  if((double)(currentTime - computers.lastXMLupdate) < Configurator::instance()->xmlRefreshLimit){
    return(0);
  }
  
  std::string tempFilename = Configurator::xmlDir + computers.getOutputDirectory() + "temp.xml";
  std::string activeFilename = Configurator::xmlDir + computers.getOutputDirectory();
  activeFilename.append(file);
  
  std::fstream tempFileStream;
  tempFileStream.open(tempFilename.c_str(), std::ios::out | std::ios::trunc);
  
  if(!tempFileStream){
    fprintf(stderr, "Temporary XML document could not be opened: %s\n", tempFilename.c_str());
    return(1);
  }
  
  tempFileStream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
  tempFileStream << "<computers>\n";
  
  for (auto it = all_computers.begin(); it != all_computers.end(); ++it) {
    // Skip computers when frequency is 0
    if ((*it)->get_freq() == 0) {
      continue;
    }

    /// <computer>
    tempFileStream << "\t<computer>\n";
    
    /// <ip>
    tempFileStream << "\t\t<ip>" << (*it)->get_address() << "</ip>\n";
    
    /// <freq>
    tempFileStream << "\t\t<frequency>" << (*it)->get_freq() << "</frequency>\n";
    
    /// <packets>
    tempFileStream << "\t\t<packets>" << (*it)->get_packets_count() << "</packets>\n";
    
    /// <date>
    const time_t last = (*it)->get_last_packet_time();
    tempFileStream << "\t\t<date>" << ctime(&last) << "</date>\n";
    
    /// <skews>
    tempFileStream << "\t\t<skews>\n";
    
    tempFileStream.setf(std::ios::fixed);
    tempFileStream.precision(6);
    /// <skew>
    // resp. <skew val=x from=y to=z>
    for (auto iter = ((*it)->timeSegmentList).cbegin(); iter != ((*it)->timeSegmentList).cend(); ++iter){
      tempFileStream << "\t\t\t<skew value=\""  << iter->alpha << "\" ";
      tempFileStream << "from=\"" << iter->relativeStartTime << "\" ";
      tempFileStream << "to=\"" << iter->relativeEndTime << "\"/>\n";    
    }
    tempFileStream.unsetf(std::ios::fixed);
    tempFileStream << "\t\t</skews>\n";
    
    // find computers with similar clock skew
    identity_container similar_skew = computers.get_similar_identities((*it)->get_address());
    
    for (auto skew_it = similar_skew.begin(); skew_it != similar_skew.end(); ++skew_it) {
      /// <identity>
      tempFileStream << "\t\t<identity>\n";
      /// <name>
      tempFileStream << "\t\t\t<name>"  << *skew_it << "</name>\n";
	  tempFileStream << "\t\t</identity>\n";
    }
    tempFileStream << "\t</computer>\n";
  }
  tempFileStream << "</computers>\n";
  tempFileStream.close();

  std::ifstream activeFileStream(activeFilename.c_str());
  // active.xml exists and has to be removed
  if (activeFileStream.good()) {
    if (remove(activeFilename.c_str())) {
      fprintf(stderr, "XML document could not be removed: %s\n", tempFilename.c_str());
      return (1);
    }
  }
  
  // rename temp.xml to active.xml
  if (rename(tempFilename.c_str(), activeFilename.c_str())) {
    fprintf(stderr, "XML document could not be replaced by temporary file: %s\n", tempFilename.c_str());
    return (1);
  }
  // update time of last xml refresh
  time(&(computers.lastXMLupdate));
  return (0);
}
