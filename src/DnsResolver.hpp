/*
 * dns-latency-monitor
 *
 * Chiara Orsini
 * chiara@caida.org
 *
 * This file is part of dns-latency-monitor.
 *
 * dns-latency-monitor is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * dns-latency-monitor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with dns-latency-monitor.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _DNSRESOLVER_H
#define _DNSRESOLVER_H

#include <iostream>
#include <ldns/ldns.h>
#include "dns_latency_monitor-config.h"

#if defined(HAVE_PTHREAD_H) && HAVE_PTHREAD_H == 1
#include <pthread.h>
#endif

/* Dns resolver:
 * this class is a wrapper around the ldns dns querying functionalities
 * query_nameserver query the domain_name provided and returns the
 * latency in milliseconds
 */
class DnsResolver{
private:
  ldns_resolver * resolver;
  // get current utc time
  void current_utc_time(struct timespec *ts);
  /* the query-latency measurements is in a 
   * critical section in order to have a more accurate latency
   * i.e. no other thread using the dnsresolver can
   * query another server at the same moment */
#if defined(HAVE_PTHREAD_H) && HAVE_PTHREAD_H == 1
  pthread_mutex_t measuring_mutex;
#endif
public:
  DnsResolver();
  double query_nameserver(const std::string domain_name);
  ~DnsResolver();
};

#endif /* _DNSRESOLVER_H */


 
