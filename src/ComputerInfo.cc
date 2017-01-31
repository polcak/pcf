/**
 * Copyright (C) 2012 Jakub Jirasek <xjiras02@stud.fit.vutbr.cz>
 *                    Libor Polčák <ipolcak@fit.vutbr.cz>
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

#include <cassert>
#include <cmath>
#include <cstring>
#include <list>
#include <iostream>
#include <sstream>
#include <fstream>

#include "ComputerInfoList.h"
#include "TimeSegment.h"
#include "Computations.h"
#include "PacketTimeInfo.h"
#include "Point.h"
#include "TimeSegmentList.h"
#include "Configurator.h"
#include "ComputerInfo.h"
#include "check_computers.h"

const size_t STRLEN_MAX = 100;

const double SKEW_VALID_AFTER = 5 * 60;

ComputerInfo::ComputerInfo(void * parentList, const char* its_address, uint16_t its_port) :
packets(), freq(0), confirmedSkew(UNDEFINED_SKEW, UNDEFINED_SKEW), packetSegmentList(),
ipAddress(its_address), port(its_port), variance(0), avg(0), numOfPackets(0), sum1(0), sum2(0),
oneMoreHour(0), firstPacketReceived(false) {
  this->parentList = parentList;
  if (!Configurator::instance()->portEnable) {
    address = ipAddress;
  } else {
    std::stringstream buffer;
    buffer << ipAddress << '_' << port;
    address = buffer.str();
  }
}

void ComputerInfo::insert_first_packet(double packet_delivered, uint32_t timestamp) {
  firstPacketReceived = true;
  lastPacketTime = packet_delivered;
  lastConfirmedPacketTime = packet_delivered;
  startTime = packet_delivered;
  previousPacketTime = startTime;

  insert_packet(packet_delivered, timestamp);
  add_empty_packet_segment(packets.begin());
}

void ComputerInfo::insert_packet(double packet_delivered, uint32_t timestamp) { // This method shouldn't suppose that skew_list contain valid information
  PacketTimeInfo new_packet;

  new_packet.ArrivalTime = packet_delivered;
  new_packet.Timestamp = timestamp;

  if (freq != 0) {
    Computations::SetOffset(new_packet, *(packets.begin()), freq);
  }

  packets.push_back(new_packet);
  lastPacketTime = packet_delivered;

#ifdef PACKETS
  printf("Time: %.6lf\n", new_packet.ArrivalTime);
  printf("Timestamp: %lu\n\n", new_packet.Timestamp);
#endif
}

bool ComputerInfo::check_block_finish(double packet_delivered) {
  bool retval = false;
  //recompute_block(packet_delivered);
  if ((Configurator::instance()->setFreq != 0) ||
      ((get_packets_count() % Configurator::instance()->block) == 0) ||
      ((packet_delivered - lastConfirmedPacketTime) > SKEW_VALID_AFTER)) {
    recompute_block(packet_delivered);
    retval = true;
  }

  return retval;
}

void ComputerInfo::recompute_block(double packet_delivered) {
  if (freq == 0 && Configurator::instance()->setFreq != 0) {
    freq = Configurator::instance()->setFreq;
  }
  // Set frequency
  if (freq == 0) {
    if ((packet_delivered - startTime) < 60) {
      return;
    } else {
      freq = compute_freq();
#if 0
      fprintf(stderr, "Found %s with frequency %d", address.c_str(), freq);
#endif

      const PacketTimeInfo &first = *(packets.begin());
      if (freq != 0) {
        for (packetTimeInfoList::iterator it = packets.begin(); it != packets.end(); ++it) {
          Computations::SetOffset(*it, first, freq);
        }
      } else {
        return;
      }
    }
  }

  /// Save Offsets into file
  save_packets(1);
  //std::cout << "packets saved " << address << std::endl;

  /// Recompute skew for graph
  PacketSegment &last_skew = *packetSegmentList.rbegin();
  ClockSkewPair new_skew = compute_skew(last_skew.first, packets.end());
  if (std::isnan(new_skew.Alpha)) {
#ifdef DEBUG
    fprintf(stderr, "Clock skew not set for %s\n", address.c_str());
#endif
    return;
  }
#ifdef DEBUG
  printf("%s: last skew (%g, %g), new skew (%g, %g), confirmed (%g, %g)\n",
      address.c_str(), last_skew.alpha, last_skew.beta, new_skew.Alpha,
      new_skew.Beta, confirmedSkew.Alpha, confirmedSkew.Beta);
#endif
  //std::cout << packet_delivered - startTime << "\t" << last_skew.alpha << std::endl;
  if (Configurator::instance()->bashOutput) {
    std::cout << std::fixed;
    std::cout << last_skew.alpha << std::endl;
  }
  if(Configurator::instance()->setFreq != 0){
    std::ofstream outfile;
    std::string type = static_cast<ComputerInfoList *> (parentList)->getOutputDirectory();
    type.pop_back();
    outfile.open(address + "-" + type + ".dat", std::ofstream::app);
    outfile << packet_delivered - startTime << "\t" << last_skew.alpha << std::endl;
  }
  //std::cout << packet_delivered - startTime << "\t" << last_skew.alpha << std::endl;

  // update 
  if (Configurator::instance()->setFreq != 0 &&
      Configurator::instance()->setSkew != std::numeric_limits<double>::infinity()) {
    if (fabs(last_skew.alpha - Configurator::instance()->setSkew) <= 0.001) {
      if (oneMoreHour == 0) {
        oneMoreHour = packet_delivered;
        computedSkew = last_skew.alpha;
      } else {
        if ((packet_delivered - oneMoreHour) > 3600) {
          variance = (sum2 - (sum1 * sum1) / numOfPackets) / (numOfPackets);
          if(Configurator::instance()->outFile != "") {
            std::ofstream resultFile;
            resultFile.open(Configurator::instance()->outFile, std::ofstream::app);
            std::string type = static_cast<ComputerInfoList *> (parentList)->getOutputDirectory();
            type.pop_back();
            resultFile << address << "\t" << type << "\t" << freq << "\t" << Configurator::instance()->setSkew << "\t" <<
                computedSkew << "\t" << last_skew.alpha << "\t" << numOfPackets << "\t" <<
                oneMoreHour - startTime << "\t" << avg << "\t" << variance << "\t" << sqrt(variance) << std::endl;
          }
          else {
            std::cout << "target skew:\t\t" << Configurator::instance()->setSkew << std::endl;
            std::cout << "computed skew (+-1ppm):\t" << computedSkew << std::endl;
            std::cout << "last skew:\t\t" << last_skew.alpha << std::endl;
            std::cout << "num of packets:\t\t" << numOfPackets << std::endl;
            std::cout << "time:\t\t\t" << oneMoreHour - startTime << std::endl;
            std::cout << "average:\t\t" << avg << std::endl;
            std::cout << "variance:\t\t" << variance << std::endl;
            std::cout << "std deviation:\t\t" << sqrt(variance) << std::endl;
          }
          exit(EXIT_SUCCESS);
        }
      }
    }
    else {
      numOfPackets++;
      avg = (avg + (packet_delivered - startTime)) / numOfPackets;
      sum1 += packet_delivered - previousPacketTime;
      sum2 += pow(packet_delivered - previousPacketTime, 2);
      previousPacketTime = packet_delivered;
    }
  }

  last_skew.alpha = new_skew.Alpha;
  last_skew.beta = new_skew.Beta;

  if ((packet_delivered - lastConfirmedPacketTime) > SKEW_VALID_AFTER) {
    ClockSkewPair last_skew_pair = compute_skew(last_skew.confirmed, packets.end());
    if ((std::fabs(last_skew_pair.Alpha - confirmedSkew.Alpha) < 10 * Configurator::instance()->threshold) ||
        (std::isnan(confirmedSkew.Alpha))) {
      // New skew confirmed
      confirmedSkew.Alpha = new_skew.Alpha;
      confirmedSkew.Beta = new_skew.Beta;
      last_skew.confirmedAlpha = new_skew.Alpha;
      last_skew.confirmedBeta = new_skew.Beta;
      last_skew.confirmed = last_skew.last;
      last_skew.last = --packets.end();
      lastConfirmedPacketTime = packet_delivered;
#ifdef DEBUG
      printf("%s: New skew confirmed (%g, %g), time %g\n", address.c_str(),
          confirmedSkew.Alpha, confirmedSkew.Beta, last_skew.last->Offset.x);
#endif
      if (Configurator::instance()->reduce)
        if (packets.size() > (unsigned int) (Configurator::instance()->block * 15))
          reduce_packets(last_skew.first, last_skew.confirmed);
    } else {
      add_empty_packet_segment(--packets.end());
      confirmedSkew.Alpha = UNDEFINED_SKEW;
      confirmedSkew.Beta = UNDEFINED_SKEW;
      lastConfirmedPacketTime = packet_delivered;
      if (Configurator::instance()->reduce)
        reduce_packets(last_skew.first, last_skew.last);
    }
  }

  TimeSegmentList s;
  for (std::list<PacketSegment>::iterator it = packetSegmentList.begin(); it != packetSegmentList.end(); ++it) {
    TimeSegment atom = {
      it->confirmedAlpha, it->confirmedBeta,
      (it->first)->Offset.x + get_start_time(),
      (it->last)->Offset.x + get_start_time(),
      // relative start and end time
      (it->first)->Offset.x,
      (it->last)->Offset.x
    };
    if (!std::isnan(atom.alpha) && !isnan(atom.beta)) {
      s.add_atom(atom);
    }
  }
  s.set_end_time(packets.rbegin()->Offset.x + get_start_time());
  NewTimeSegmentList = s;
}

void ComputerInfo::reduce_packets(packet_iterator start, packet_iterator end) {
  packet_iterator current = start;
  current++;
  // no packets to reduce
  if (current == packets.end() || current == end) {
    return;
  }

  bool reduceMe = false;

  // there are some packets between start and end
  while (current != packets.end() && current != end) {
    packet_iterator prev = current;
    prev--;
    packet_iterator next = current;
    next++;
    if (next == end || next == packets.end()) {
      // last packet can't be reduced
      break;
    }

    // current packet doesn't affect direction of skew
    if ((prev->Offset.y > current->Offset.y) && (current->Offset.y < next->Offset.y))
      reduceMe = true;

      // 
    else if ((prev->Offset.y <= current->Offset.y) && (current->Offset.y < next->Offset.y)) {
      double tan_curr = (current->Offset.y - prev->Offset.y) / (current->Offset.x - prev->Offset.x);
      double tan_next = (next->Offset.y - prev->Offset.y) / (next->Offset.x - prev->Offset.x);
      if (tan_curr <= tan_next) {
        reduceMe = true;
      }
    }      // check here
    else if ((prev->Offset.y > current->Offset.y) && (current->Offset.y >= next->Offset.y)) {
      double tan_curr = (current->Offset.y - next->Offset.y) / (current->Offset.x - next->Offset.x);
      double tan_next = (prev->Offset.y - next->Offset.y) / (next->Offset.x - prev->Offset.x);
      if (tan_curr <= tan_next) {
        reduceMe = true;
      }
    }

    if (reduceMe) {
      current = packets.erase(current);
      // check previous packet again
      current--;
      if (current == start) {
        current++;
      }
    } else {
      current++;
    }
  }
}

void ComputerInfo::restart(double packet_delivered, uint32_t timestamp) {
  packets.clear();
  freq = 0;
  lastPacketTime = packet_delivered;
  startTime = packet_delivered;
  packetSegmentList.clear();
  insert_packet(packet_delivered, timestamp);
  add_empty_packet_segment(packets.begin());
}

void ComputerInfo::add_empty_packet_segment(packetTimeInfoList::iterator start) {
  PacketSegment skew;
  skew.alpha = UNDEFINED_SKEW;
  skew.beta = UNDEFINED_SKEW;
  skew.confirmedAlpha = UNDEFINED_SKEW;
  skew.confirmedBeta = UNDEFINED_SKEW;
  skew.first = start;
  skew.confirmed = start;
  skew.last = --packets.end();
  packetSegmentList.push_back(skew);
#ifdef DEBUG
  printf("%s: New empty skew first: %g, confirmed %g, last: %g\n", address.c_str(), (skew.first)->Offset.x, (skew.confirmed)->Offset.x, (skew.last)->Offset.x);
#endif
}

ClockSkewPair ComputerInfo::compute_skew(const packet_iterator &start, const packet_iterator &end) {
  // Prepare an array of all points for convex hull computation
  unsigned long pckts_count = get_packets_count();
  Point points[pckts_count];
  const PacketTimeInfo &first = *(start);
  ClockSkewPair result(UNDEFINED_SKEW, UNDEFINED_SKEW);

  /// First point
  points[0].x = first.Offset.x;
  points[0].y = first.Offset.y;

  unsigned long i = 1;
  auto it = start;
  if (it == packets.end()) {
    return result;
  } else {
    ++it;
    if (it == packets.end()) {
      return result;
    }
  }
  for (; (it != end) && (it != packets.end()); ++it) {
    points[i].x = it->Offset.x;
    points[i].y = it->Offset.y;
    i++;
  }
  pckts_count = i;

  // Compute upper convex hull, note that points are destroyed inside the function
  // and pckts_count will refer to the number of points in the convex hull when
  // the function finish
  Point *hull = Computations::ConvexHull(points, &pckts_count);

  // alpha is tangent of the line, beta is the Offset
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
    min += alpha * it->Offset.x + beta - it->Offset.y;
  }

  // Store computed alpha, beta; it may change if other sectors of convex hull are part
  // of the line with minimal distance
  result.Alpha = alpha;
  result.Beta = beta;

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
      sum += (alpha * it->Offset.x + beta - it->Offset.y);
      if (sum >= min) // The sum is already higher than the sum of all points of other sectors
        break;
    }

#ifdef DEBUG
    printf("[%lf,%lf],[%lf,%lf], f(x) = %lf*x + %lf, sum = %lf\n", hull[i - 1].x, hull[i - 1].y, hull[i].x, hull[i].y, alpha, beta, sum);
#endif

    // A new min was fuound, update alpha, beta, and min
    if (sum < min) {
      result.Alpha = alpha;
      result.Beta = beta;
      min = sum;
    }
  }

#ifdef DEBUG
  printf("f(x) = %lfx + %lf, min = %lf\n", result.Alpha, result.Beta, min);
#endif

  return result;
}

int ComputerInfo::compute_freq() {
  assert(!packets.empty());

  const PacketTimeInfo &first = *packets.begin();

  double tmp = 0.0;
  int count = 0;

  for (auto it = ++packets.begin(); it != packets.end(); ++it) {
    double local_diff = it->ArrivalTime - first.ArrivalTime;
    if (local_diff > 60.0) {
      tmp += ((it->Timestamp - first.Timestamp) / local_diff);
      count++;
    }
  }

  if (count < 10) {
    // Wait for more packets
    return 0;
  }

  /// According to the real world, but sometimes can be wrong, it depends...
  int freq = (int) round(tmp / count);
  if (freq >= 970 && freq <= 1030)
    freq = 1000;
  else if (freq >= 95 && freq <= 105)
    freq = 100;
  else if (freq >= 230 && freq <= 270)
    freq = 250;

  if (Configurator::instance()->verbose) {
    printf("Frequency of %s (Hz): %d\n", address.c_str(), freq);
  }
  if (Configurator::instance()->bashOutput) {
    std::cout << " " << freq << std::endl;
  }
  return freq;
}

int ComputerInfo::save_packets(short rewrite) {
  FILE *f;
  char filename[80] = "log/";
  std::strcat(filename, static_cast<ComputerInfoList *> (parentList)->getOutputDirectory().c_str());
  std::strcat(filename, get_address().c_str());
  std::strcat(filename, ".log");

  /// Open file
  if (rewrite == 1)
    f = fopen(filename, "w");
  else
    f = fopen(filename, "a");

  if (f == NULL) {
    fprintf(stderr, "Cannot save packets into the file: %s\n", filename);
    return (2);
  }

  /// Write to file
  char str[STRLEN_MAX];
  for (auto it = packets.begin(); it != packets.end(); ++it) {
    snprintf(str, STRLEN_MAX, "%lf\t%lf\t%lf\t%u\n", it->Offset.x, it->Offset.y, it->ArrivalTime, it->Timestamp);
    fputs(str, f);
  }

  /// Close file
  if (fclose(f) != 0)
    fprintf(stderr, "Cannot close file: %s\n", filename);
  return (0);
}
