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

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "list.h"


/**
 * Point
 */
typedef struct point2d point2d;

struct point2d {
  double x;
  double y;
};


/**
 * Swap two points
 * @param[in] x, @param[in] y Points
 */
void swap(point2d *x, point2d *y);

/**
 * Compute upper bound as a upper convex hull. Graham scan algorithm is used.
 * Note that the input list with points is destroyed during the process and
 * the function returns a pointer to the input list.
 * @param[in] points Points list
 * @param[in,out] number List size
 * @return Pointer to list with convex hull points
 */
point2d *convex_hull(point2d points[], unsigned long *number);

/**
 * Counter-clockwise test
 * 
 * Source: http://en.wikipedia.org/wiki/Graham_scan
 * 
 * @param[in] p1, @param[in] p2, @param[in] p3 Points
 * @return > 0 if counter-clockwise, < 0 if clockwise, = 0 collinear
 */
double ccw(point2d p1, point2d p2, point2d p3);


void swap(point2d *x, point2d *y)
{
   point2d tmp;
   tmp = *x;
   *x = *y;
   *y = tmp;
}

double ccw(point2d p1, point2d p2, point2d p3)
{
  return((p2.x - p1.x) * (p3.y - p1.y) - (p2.y - p1.y) * (p3.x - p1.x));
}

// More details in:
// Graham, R. L.: An efficient algorithm for determining the convex hull of a finite
// planar set. Information Processing Letters, vol. 1, no. 4, jan 1972: pp. 132–133,
// ISSN 0020-0190.
point2d *convex_hull(point2d points[], unsigned long *number)
{
  unsigned long i;
  int m = 1;
  
  for (i = 2; i < *number; i++) {
    while (ccw(points[m - 1], points[m], points[i]) >= 0) {
      if (m == 1) {
        swap(&points[m], &points[i]);
        i++;
      }
      else
        m--;
    }
    m++;
    swap(&points[m], &points[i]);
  }
  
  *number = ++m;
  
  return(points);
}


int compute_clock_frequency(packet_time_info *packets)
{
  assert(packets != NULL);
  
  packet_time_info *first = packets;

  double tmp = 0.0;
  int count = 0;
  
  for (packet_time_info *current = first->next_packet; current != NULL; current = current->next_packet) {
    double local_diff = current->time - first->time;
    if (local_diff > 60.0) {
      tmp += ((current->timestamp - first->timestamp) / local_diff);
      count++;
    }
  }

	if (count < 10) {
		// Wait for more packets
		return 0;
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

void set_offsets(packet_time_info *head_packet, packet_time_info *from, int freq)
{
  packet_time_info *current;
  double tmp;
  
  for (current = from; current != NULL; current = current->next_packet) {
    current->offset.x = current->time - head_packet->time;
    
    tmp = current->timestamp - head_packet->timestamp;
    tmp /= freq;
    tmp -= current->offset.x;
    
    // s -> ms
    tmp *= 1000;
    
    current->offset.y = tmp;
  }
}

int set_skew(computer_info *list)
{
	// Prepare an array of all points for convex hull computation
  unsigned long pckts_count = packets_count(list->head_packet);
  point2d points[pckts_count];
  packet_time_info *current = list->head_packet;
  
  /// First point
  points[0].x = current->offset.x;
  points[0].y = current->offset.y;
  
  unsigned long i = 1;
  for (current = current->next_packet; current != NULL; current = current->next_packet) {
    points[i].x = current->offset.x;
    points[i].y = current->offset.y;
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
  
  if (fabs(alpha) > 100)
    return(1);
  
  beta = hull[j - 1].y - (alpha * hull[j - 1].x);
  min = 0.0;
  for (current = list->head_packet; current != NULL; current = current->next_packet) {
    min += alpha * current->offset.x + beta - current->offset.y;
  }

	// Store computed alpha, beta; it may change if other sectors of convex hull are part
	// of the line with minimal distance
  list->skew.alpha = alpha;
  list->skew.beta = beta;
  
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
    for (current = list->head_packet; current != NULL; current = current->next_packet) {
      sum += (alpha * current->offset.x + beta - current->offset.y);
      if (sum >= min) // The sum is already higher than the sum of all points of other sectors
        break;
    }
    
#ifdef DEBUG
    printf("[%lf,%lf],[%lf,%lf], f(x) = %lf*x + %lf, sum = %lf\n", hull[i - 1].x, hull[i - 1].y, hull[i].x, hull[i].y, alpha, beta, sum);
#endif
    
    // A new min was fuound, update alpha, beta, and min
    if (sum < min) {
      list->skew.alpha = alpha;
      list->skew.beta = beta;
      min = sum;
    }
  }
  
#ifdef DEBUG
  printf("f(x) = %lfx + %lf, min = %lf\n", list->skew.alpha, list->skew.beta, min);
#endif
  
  return(0);
}