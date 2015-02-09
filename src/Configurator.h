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

#ifndef _CONFIGURATOR_H
#define _CONFIGURATOR_H

#include <string>

/**
 * Structure with all config data
 */

class Configurator {
private:
    static Configurator * innerInstance;
    
public:
  Configurator() {};
  
  int icmpDisable;
  bool javacriptDisable;
  bool tcpDisable;
  bool portEnable;
  bool verbose;
  bool exportSkewChanges;
  std::string datafile;
  bool reduce;
    
  char dev[10];
  std::string datalink;
  int number;
  unsigned int time;
  u_int16_t port;
  char src[16];
  char dst[16];
  short syn;
  short ack;
  char filter[4096];
  
  char active[1024];
  char database[1024];
  static const std::string xmlDir;
  
  int block;
  int timeLimit;
  double threshold;
  double xmlRefreshLimit;
  
  double setFreq;
  bool bashOutput;
  double setSkew;
  
  void Init();

  /**
   * Fill the config structure
   * @param[in] filename          Config file name
   */
  void GetConfig(const char *filename);
  
  static Configurator * instance();
};


#endif
