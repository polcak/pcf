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

#include <cassert>
#include <cmath>
#include <cstring>
#include <list>

#include "clock_skew.h"
#include "clock_skew_guard.h"
#include "computations.h"
#include "packet_time_info.h"
#include "point2d.h"
#include "skew.h"

const size_t STRLEN_MAX = 100;

const double SKEW_VALID_AFTER = 5*60;

computer_info::computer_info(double first_packet_delivered, uint32_t first_packet_timstamp,
    const char* its_address, const int its_block_size):
  packets(), address(its_address), freq(0),
  last_packet_time(first_packet_delivered), last_confirmed_skew(first_packet_delivered),
  confirmed_skew(UNDEFINED_SKEW, UNDEFINED_SKEW),
  start_time(first_packet_delivered), skew_list(), block_size(its_block_size)
{
  insert_packet(first_packet_delivered, first_packet_timstamp);
  add_empty_skew(packets.begin());
}



void computer_info::insert_packet(double packet_delivered, uint32_t timestamp)
{ // This method shouldn't suppose that skew_list contain valid information
  packet_time_info new_packet;

  new_packet.time = packet_delivered;
  new_packet.timestamp = timestamp;

  if (freq != 0) {
    set_offset(new_packet, *(packets.begin()), freq);
  }

  packets.push_back(new_packet);
  last_packet_time = packet_delivered;

#ifdef PACKETS
  printf("Time: %.6lf\n", new_packet.time);
  printf("Timestamp: %lu\n\n", new_packet.timestamp);
#endif
}

void computer_info::insert_packet(double packet_delivered, uint32_t timestamp, clock_skew_guard &skews)
{
  insert_packet(packet_delivered, timestamp);

  if (((get_packets_count() % block_size) == 0) ||
      ((packet_delivered - last_confirmed_skew) > SKEW_VALID_AFTER)) {
    block_finished(packet_delivered, skews);
  }
}



void computer_info::block_finished(double packet_delivered, clock_skew_guard &skews)
{
  // Set frequency
  if (freq == 0) {
    if ((packet_delivered - start_time) < 60) {
      return;
    }
    else {
      freq = compute_freq();
#if 0
      fprintf(stderr, "Found %s with frequency %d", address.c_str(), freq);
#endif

      const packet_time_info &first = *(packets.begin());
      if (freq != 0) {
        for (auto it = packets.begin(); it != packets.end(); ++it) {
          set_offset(*it, first, freq);
        }
      }
      else {
        return;
      }
    }
  }

  /// Save offsets into file
  save_packets(1);

  /// Recompute skew for graph
  skew_info &last_skew = *skew_list.rbegin();
  clock_skew_pair new_skew = compute_skew(last_skew.first, packets.end());
  if (std::isnan(new_skew.first)) {
#ifdef DEBUG
    fprintf(stderr, "Clock skew not set for %s\n", address.c_str());
#endif
    return;
  }
#ifdef DEBUG
  printf("%s: last skew (%g, %g), new skew (%g, %g), confirmed (%g, %g)\n",
      address.c_str(), last_skew.alpha, last_skew.beta, new_skew.first,
      new_skew.second, confirmed_skew.first, confirmed_skew.second);
#endif

  last_skew.alpha = new_skew.first;
  last_skew.beta = new_skew.second;

  if ((packet_delivered - last_confirmed_skew) > SKEW_VALID_AFTER) {
    clock_skew_pair last_skew_pair = compute_skew(last_skew.confirmed, packets.end());
#ifdef DEBUG
    printf("%s: clock skew in last period (%g, %g)\n",
        address.c_str(), last_skew_pair.first, last_skew_pair.second);
#endif
    if ((std::fabs(last_skew_pair.first - confirmed_skew.first) < 10*skews.get_threshold()) ||
        (std::isnan(confirmed_skew.first))) {
      // New skew confirmed
      confirmed_skew.first = new_skew.first;
      confirmed_skew.second = new_skew.second;
      last_skew.confirmed = last_skew.last;
      last_skew.last = --packets.end();
      last_confirmed_skew = packet_delivered;
#ifdef DEBUG
      printf("%s: New skew confirmed (%g, %g), time %g\n", address.c_str(),
          confirmed_skew.first, confirmed_skew.second, last_skew.last->offset.x);
#endif

#ifndef DONOTREDUCE
      // Reduce packets
      if (packets.size() > (block_size * 5)) {
        reduce_packets(last_skew.first, last_skew.confirmed);
      }
#endif
    }
    else {
#ifndef DONOTREDUCE
      // Reduce packets
      reduce_packets(last_skew.first, last_skew.confirmed);
#endif

      add_empty_skew(--packets.end());
      confirmed_skew.first = UNDEFINED_SKEW;
      confirmed_skew.second = UNDEFINED_SKEW;
      last_confirmed_skew = packet_delivered;
    }
  }

  skew s;
  for (auto it = skew_list.begin(); it != skew_list.end(); ++it) {
    clock_skew_atom atom = {it->alpha, it->beta,
      (it->first)->offset.x + get_start_time(),
      (it->last)->offset.x + get_start_time(),
      (it->first)->offset.x,
      (it->last)->offset.x
    };
    if (!std::isnan(atom.alpha) && !isnan(atom.beta)) {
      s.add_atom(atom);
    }
  }
  s.set_end_time(packets.rbegin()->offset.x + get_start_time());
  skews.update_skew(address, s);
}



void computer_info::restart(double packet_delivered, uint32_t timestamp)
{
  packets.clear();
  freq = 0;
  last_packet_time = packet_delivered;
  start_time = packet_delivered;
  skew_list.clear();
  insert_packet(packet_delivered, timestamp);
  add_empty_skew(packets.begin());
}



void computer_info::add_empty_skew(packet_time_info_list::iterator start)
{
  skew_info skew;
  skew.alpha = UNDEFINED_SKEW;
  skew.beta = UNDEFINED_SKEW;
  skew.first = start;
  skew.confirmed = start;
  skew.last = --packets.end();
  skew_list.push_back(skew);
#ifdef DEBUG
  printf("%s: New empty skew first: %g, confirmed %g, last: %g\n", address.c_str(), (skew.first)->offset.x, (skew.confirmed)->offset.x, (skew.last)->offset.x);
#endif
}



void computer_info::reduce_packets(packet_iterator start, packet_iterator end)
{
#ifdef DEBUG
  printf("Reduction for IP %s start: %u packets\n", address.c_str(), (unsigned int) packets.size());
#endif
  packet_iterator current = start;
  ++current;
  if ((current == packets.end()) || (current == end)) {
    return;
  }

  while ((current != packets.end()) && (current != end)) {
    packet_iterator prev = current;
    --prev;
    packet_iterator next = current;
    ++next;
    if ((next == end) || (next == packets.end())) {
      break; // We can't reduce the last packet
    }
    double prev_x = prev->offset.x;
    double curr_x = current->offset.x;
    double next_x = next->offset.x;
    double prev_y = prev->offset.y;
    double curr_y = current->offset.y;
    double next_y = next->offset.y;
    bool reduce = false;
    if ((prev_y > curr_y) && (curr_y < next_y)) {
      reduce = true;
    }
    else if ((prev_y <= curr_y) && (curr_y < next_y)) {
      double tan_curr = (curr_y - prev_y) / (curr_x - prev_x);
      double tan_next = (next_y - prev_y) / (next_x - prev_x);
      if (tan_curr <= tan_next) {
        reduce = true;
      }
    }
    else if ((prev_y > curr_y) && (curr_y >= next_y)) {
      double tan_curr = (curr_y - next_y) / (next_x - curr_x);
      double tan_prev = (prev_y - next_y) / (next_x - prev_x);
      if (tan_curr <= tan_prev) {
        reduce = true;
      }
    }

    if (reduce) {
      current = packets.erase(current);
      --current; // Check previous packet again
      if (current == start) {
        ++current; // unless it is the first one
      }
    }
    else {
      ++current;
    }
  }
#ifdef DEBUG
  printf("Reduction for IP %s end: %u packets\n", address.c_str(), (unsigned int)packets.size());
#endif
}



computer_info::clock_skew_pair computer_info::compute_skew(const packet_iterator &start, const packet_iterator &end)
{
  // Prepare an array of all points for convex hull computation
  unsigned long pckts_count = get_packets_count();
  point2d points[pckts_count];
  const packet_time_info &first = *(start);
  clock_skew_pair result(UNDEFINED_SKEW, UNDEFINED_SKEW);

  /// First point
  points[0].x = first.offset.x;
  points[0].y = first.offset.y;
  
  unsigned long i = 1;
  auto it = start;
  if (it == packets.end()) {
    return result;
  }
  else {
    ++it;
    if (it == packets.end()) {
      return result;
    }
  }
  for (; (it != end) && (it != packets.end()); ++it) {
    points[i].x = it->offset.x;
    points[i].y = it->offset.y;
    i++;
  }
  pckts_count = i-1;
  
  // Compute upper convex hull, note that points are destroyed inside the function
  // and pckts_count will refer to the number of points in the convex hull when
  // the function finish
  point2d *hull = convex_hull(points, &pckts_count);

  // alpha is tangent of the line, beta is the offset
  // y = alpha * x + beta
  double alpha, beta;

  // These two variables are used to compute the distance of a sector in the found convex hull to
  // all points. Min is the minimal distance, sum is valid for actual sector.
  double min, sum;

  // Compute j-th sector first
  unsigned long j = (pckts_count / 2);

  // Compute alpha, beta, min for the j-th sector (sum is not used atm)
  alpha = ((hull[j].y - hull[j - 1].y) / (hull[j].x - hull[j - 1].x));

  if (std::fabs(alpha) > 100) {
    return result;
  }

  beta = hull[j - 1].y - (alpha * hull[j - 1].x);
  min = 0.0;
  for (auto it = start; (it != end) && (it != packets.end()); ++it) {
    min += alpha * it->offset.x + beta - it->offset.y;
  }

  // Store computed alpha, beta; it may change if other sectors of convex hull are part
  // of the line with minimal distance
  result.first = alpha;
  result.second = beta;
  
#ifdef DEBUG
  printf("\n");
  printf("[%lf,%lf],[%lf,%lf], f(x) = %lf*x + %lf, sum = %lf\n", hull[j - 1].x, hull[j - 1].y, hull[j].x, hull[j].y, alpha, beta, min);
#endif

  // Compute alpha, beta, sum for other sectors
  for (i = 1; i < pckts_count; i++) {
    if (i == j) // We already computed j-th sector
      continue;
    
    alpha = ((hull[i].y - hull[i - 1].y) / (hull[i].x - hull[i - 1].x));
    
    /// Too steep
    if (alpha > 3 || alpha < -3)
      continue;
    
    beta = hull[i - 1].y - (alpha * hull[i - 1].x);
    
    /// SUM
    sum = 0.0;
    for (auto it = start; (it != end) && (it != packets.end()); ++it) {
      sum += (alpha * it->offset.x + beta - it->offset.y);
      if (sum >= min) // The sum is already higher than the sum of all points of other sectors
        break;
    }
    
#ifdef DEBUG
    printf("[%lf,%lf],[%lf,%lf], f(x) = %lf*x + %lf, sum = %lf\n", hull[i - 1].x, hull[i - 1].y, hull[i].x, hull[i].y, alpha, beta, sum);
#endif
    
    // A new min was fuound, update alpha, beta, and min
    if (sum < min) {
      result.first = alpha;
      result.second = beta;
      min = sum;
    }
  }
  
#ifdef DEBUG
  printf("f(x) = %lfx + %lf, min = %lf\n", result.first, result.second, min);
#endif
  
  return result;
}



int computer_info::compute_freq()
{
  assert(!packets.empty());

  if (get_packets_count() < 10) {
    // Wait for more packets
    return 0;
  }

  const packet_time_info &first = *packets.begin();

  double tmp = 0.0;
  int count = 0;

  for (auto it = ++packets.begin(); it != packets.end(); ++it) {
    double local_diff = it->time - first.time;
    if (local_diff > 60.0) {
      tmp += ((it->timestamp - first.timestamp) / local_diff);
      count++;
    }
  }

  /// According to the real world, but sometimes can be wrong, it depends...
  int freq = (int)round(tmp / count);
  if (freq >= 970 && freq <= 1030)
    freq = 1000;
  else if (freq >= 95 && freq <= 105)
    freq = 100;
  else if (freq >= 230 && freq <= 270)
    freq = 250;
  
#ifdef DEBUG
  printf("Frequency (Hz): %d\n", freq);
#endif
  
  return freq;
}



int computer_info::save_packets(short rewrite)
{
  FILE *f;
  char filename[50] = "log/";
  std::strcat(filename, get_address().c_str());
  std::strcat(filename, ".log");
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

  /// Write to file
  char str[STRLEN_MAX];
  for (auto it = packets.begin(); it != packets.end(); ++it) {
    snprintf(str, STRLEN_MAX, "%lf\t%lf\n", it->offset.x, it->offset.y);
    fputs(str, f);
#ifdef DEBUG
    lines++;
#endif
  }
  
  /// Close file
  if (fclose(f) != 0)
    fprintf(stderr, "Cannot close file: %s\n", filename);

#ifdef DEBUG
    fprintf(stderr, "%s: %d lines written", get_address().c_str(), lines);
#endif
  
  return(0);
}
