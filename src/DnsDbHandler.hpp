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

#ifndef _DNSDBHANDLER_H
#define _DNSDBHANDLER_H

#include <iostream>
#include <vector>
#include <mysql++.h>

#include "dns_latency_monitor-config.h"

#if defined(HAVE_PTHREAD_H) && HAVE_PTHREAD_H == 1
#include <pthread.h>
#endif



/* DnsDbHandler:
 * this class provides two main features
 * - it manages the connection the mysql database (and the concurrency)
 * - it updates the statistics per domain using incremental
 *   avg and stdev computation
 */
class DnsDbHandler{
private:
  mysqlpp::Connection db_conn;
  // 1 thread at the time can use th db_connection
#if defined(HAVE_PTHREAD_H) && HAVE_PTHREAD_H == 1
  pthread_mutex_t db_conn_mutex;
#endif
public:
  DnsDbHandler(const char * db_name,
	       const char * server = NULL,
	       const char * user = NULL,
	       const char * password = NULL,
	       const char * socket = NULL,
	       unsigned int port = 0
	       );
  std::map<int,std::string> get_top_n_domains(unsigned int n = 10);
  void update_dns_stats(int domain_id, double latency, int current_ts);
  ~DnsDbHandler();
};

#endif /* _DNSDBHANDLER_H */
 
