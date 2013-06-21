/**
 * Copyright (C) 2012 Libor Polčák <ipolcak@fit.vutbr.cz>
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

#include <cstring>
#include <ctime>
#include <string>
#include <iostream>

#include "TimeSegment.h"
#include "gnuplot_graph.h"
#include "Configurator.h"

const size_t STRLEN_MAX = 100;

void time_to_str(char *buffer, size_t buffer_size, time_t time)
{
  struct tm time_data = *localtime(&time);
  strftime(buffer, buffer_size, "%Y-%m-%d %H:%M:%S", &time_data);
}

void gnuplot_graph::Notify(const AnalysisInfo& changed_skew)
{
  if(Configurator::instance()->debug)
    printf("gnuplot_graph::notify %s\n", changed_skew.Address.c_str());
  generate_graph(changed_skew);
}


void gnuplot_graph::generate_graph(const AnalysisInfo &changed_skew)
{
  const std::string& address = changed_skew.Address;
  const TimeSegmentList& computer_skew = changed_skew.ClockSkewList;

  // get output directory according to type
  std::string extendedFilenameTemplate = "graph/" + getOutputDirectory() + "%s.gp";
  const char* filename_template = extendedFilenameTemplate.c_str();
  
  FILE *f;
  int filename_max = strlen(filename_template) + address.length();
  char filename[filename_max + 1];
  snprintf(filename, filename_max, filename_template, address.c_str());
  f = fopen(filename, "w");
  
  if (f == NULL) {
    fprintf(stderr, "Cannot create file: %s\n", filename);
    return;
  }
  
  if (computer_skew.cbegin()->alpha == 0.0 || isnan(computer_skew.cbegin()->alpha))
    return;

  const unsigned interval_count = 10;
  unsigned interval_min = computer_skew.get_start_time();
  unsigned interval_max = computer_skew.get_end_time();
  unsigned interval_size = (interval_max - interval_min) / interval_count;

  fputs("set encoding iso_8859_2\n"
        "set terminal svg\n"
        "set output 'www/graph/", f);
        fputs(getOutputDirectory().c_str(), f);
        fputs(address.c_str(), f);
        fputs(".svg'\n\n"
        "set x2label 'Date and time'\n"
        "set xlabel 'Elapsed time [s]'\n"
        "set ylabel 'Offset [ms]'\n\n"
        "set x2tics axis in rotate by 270 textcolor lt 4\n"
        "set xtics mirror 0, ", f);
  fprintf(f, "%u\n\n", interval_size);
  fputs("set datafile separator '\\t'\n\n"
        "set x2tics ("
        , f);

  char tmp[100];

  time_t boundary = interval_min;
  unsigned boundary_m = 0;
  for (unsigned int i = 0; i <= interval_count; i++) {
    char date_time[STRLEN_MAX];
    time_to_str(date_time, STRLEN_MAX, boundary);
    sprintf(tmp, "'%s' %u", date_time, boundary_m);
    fputs(tmp, f);
    boundary += interval_size;
    boundary_m += interval_size;
    if (i < interval_count) {
      fputs(", ", f);
    }
  }

  fputs (")\n\n", f);
  unsigned int count = 1;
  for (auto it = computer_skew.cbegin(); it != computer_skew.cend(); ++it, ++count) {
    if (it->relativeStartTime == it->relativeEndTime) {
      continue;
    }
    // fn(x)
    fputs ("f", f);
    sprintf(tmp, "%u", count);
    fputs (tmp, f);
    fputs ("(x) = ", f);
    // Intervals
    fputs ("(", f);
    sprintf(tmp, "%lf", it->relativeStartTime);
    fputs(tmp, f);
    fputs(" < x && x < ", f);
    sprintf(tmp, "%lf", it->relativeEndTime);
    fputs(tmp, f);
    fputs (") ? ", f);
    // alpha *x + beta
    sprintf(tmp, "%lf", it->alpha);
    fputs(tmp, f);
    fputs("*x + ", f);
    sprintf(tmp, "%lf", it->beta);
    fputs(tmp, f);
    fputs(" : 1/0\n", f);
  }

  fputs("\n\nset grid xtics x2tics ytics\n"
        "set title \"", f);
  
  /// Title
  time_t rawtime;
  time(&rawtime);
  sprintf(tmp, "%s", ctime(&rawtime));
  tmp[strlen(tmp)-1] = '\0';
  fputs(tmp, f);
  fputs("\\n", f);
  fputs(address.c_str(), f);
  fputs("\\n", f);
  fputs(type.c_str(), f);

  // Search for computers with similar skew
  identity_container similar_devices = changed_skew.SimilarIdentities;
  for (auto it = similar_devices.begin(); it != similar_devices.end(); ++it) {
    fputs("\\n", f);
    fputs(it->c_str(), f);
  }
  if (similar_devices.empty()) {
    fputs("\\nunknown\" textcolor lt 1", f);
    }
  else {
    fputs("\" textcolor lt 2", f);
  }

  /// Plot
  fputs("\n\n"
        "plot '", f);
  fputs("log/", f);
  fputs(getOutputDirectory().c_str(), f);
  fputs(address.c_str(), f);
  fputs(".log' title ''", f);

  count = 1;
  for (auto it = computer_skew.cbegin(); it != computer_skew.cend(); ++it, ++count) {
    if (it->relativeStartTime == it->relativeEndTime) {
      continue;
    }
    fputs (", f", f);
    sprintf(tmp, "%u", count);
    fputs (tmp, f);
    fputs ("(x)", f);
    /// Legend
    fputs(" title 'f", f);
    fputs (tmp, f);
    fputs("(x) = ", f);
    sprintf(tmp, "%lf", it->alpha);
    fputs(tmp, f);
    fputs("*x + ", f);
    sprintf(tmp, "%lf", it->beta);
    fputs(tmp, f);
    fputs("'", f);
  }

  fputs("\n", f);

  if (fclose(f) != 0)
    fprintf(stderr, "Cannot close file: %s\n", filename);
  
  std::string extendedGnuplotTemplate = "gnuplot graph/" + getOutputDirectory() + "%s.gp";
  const char* gnuplot_template =  extendedGnuplotTemplate.c_str();
  int gnuplot_max = strlen(gnuplot_template) + address.length();
  char gnuplot_cmd[gnuplot_max + 1];
  snprintf(gnuplot_cmd, gnuplot_max, gnuplot_template, address.c_str());
  
  //
  if(system(gnuplot_cmd) < 0)
      fprintf(stderr, "Error while launching gnuplot\n");
  
  return;
}
