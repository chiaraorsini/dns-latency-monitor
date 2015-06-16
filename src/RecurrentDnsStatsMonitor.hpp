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

#ifndef _RECURRENTDNSTATSMONITOR_H
#define _RECURRENTDNSTATSMONITOR_H

#include <iostream>
#include <stdexcept>
#include <exception>

#include "DnsDbHandler.hpp"
#include "DnsResolver.hpp"



/* Recurrent Dns Stats Monitor:
 * this class manages a DnsDbHandler and DnsResolver
 * it provides a run function that initializes a db
 * and then collect dns query latency statistics (using the DnsResolver)
 * if pthreads are present every domain is analyzed in
 * a separate thread
 * otherwise operations are performed sequentially
 */

class RecurrentDnsStatsMonitor{
private:
  DnsDbHandler ddh;
  DnsResolver dr;
  std::map<int,std::string> top_domains;
  unsigned int dns_test_frequency; // initialized during the "run"
  unsigned int max_num_cycles;    // initialized during the "run"
public:
  RecurrentDnsStatsMonitor(const char * db_name,
			   const char * server = NULL,
			   const char * user = NULL,
			   const char * password = NULL,
			   const char * socket = NULL,
			   unsigned int port = 0);
  void run(unsigned int frequency = 60, unsigned int cycles = 0);
#if defined(HAVE_PTHREAD_H) && HAVE_PTHREAD_H == 1
  void single_domain_run(int domain_id, std::string domain_name);
  void parallel_run(unsigned int frequency = 60, unsigned int cycles = 0);
#endif
  ~RecurrentDnsStatsMonitor();
};

#endif /* _RECURRENTDNSTATSMONITOR_H */


 
