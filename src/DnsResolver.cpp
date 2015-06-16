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

#include "DnsResolver.hpp"

#include <time.h>
#include <sys/time.h>
#include <stdio.h>
 
#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif
 

// solution adjusted from on https://gist.github.com/jbenet/1087739
void DnsResolver::current_utc_time(struct timespec *ts) {
#ifdef __MACH__ // OS X does not have clock_gettime, use clock_get_time
  clock_serv_t cclock;
  mach_timespec_t mts;
  host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &cclock);
  clock_get_time(cclock, &mts);
  mach_port_deallocate(mach_task_self(), cclock);
  ts->tv_sec = mts.tv_sec;
  ts->tv_nsec = mts.tv_nsec;
#else
  clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
#endif
}
 

DnsResolver::DnsResolver() {
  try{
    // create resolver structure
    ldns_status s = ldns_resolver_new_frm_file(&resolver, NULL);
    if(s != LDNS_STATUS_OK){
      throw std::string("Can't create DnsResolver()");
    }
#if defined(HAVE_PTHREAD_H) && HAVE_PTHREAD_H == 1
    pthread_mutex_init(&measuring_mutex, NULL);
#endif
  }
  catch(std::string s){
    throw s;
  }
}


double DnsResolver::query_nameserver(const std::string domain_name) { 
  ldns_rdf * domain = NULL;
  ldns_pkt * response_packet = NULL;
  long double time_diff = -1.0; 
  domain = ldns_dname_new_frm_str(domain_name.c_str());
  if(domain == NULL) {
    std::cerr << domain_name << " cannot be parsed" << std::endl;
    return -1;
  }
  // timespec - nanoseconds resolution
  struct timespec ts_before;
  struct timespec ts_after;
#if defined(HAVE_PTHREAD_H) && HAVE_PTHREAD_H == 1
  pthread_mutex_lock(&measuring_mutex);
#endif
  current_utc_time(&ts_before);
  response_packet = ldns_resolver_query(resolver,
					domain,
					LDNS_RR_TYPE_A,   // a host address
					LDNS_RR_CLASS_IN, // Internet
					LDNS_RD);         // recursion desired
  // http://www.iana.org/assignments/dns-parameters/dns-parameters.xhtml
  current_utc_time(&ts_after);
#if defined(HAVE_PTHREAD_H) && HAVE_PTHREAD_H == 1
  pthread_mutex_unlock(&measuring_mutex);
#endif
  if(response_packet == NULL) {
    std::cerr << "Query for " << domain_name << " failed" << std::endl;
  }
  else{
    // free memory allocated for packet
    ldns_pkt_free(response_packet);
    // time elapsed adjusted code from:
    // http://www.gnu.org/software/libc/manual/html_node/Elapsed-Time.html
    struct timespec temp;
    if ((ts_after.tv_nsec - ts_before.tv_nsec) < 0) {
      temp.tv_sec = ts_after.tv_sec - ts_before.tv_sec - 1;
      temp.tv_nsec = 1000000000 + ts_after.tv_nsec - ts_before.tv_nsec;
    } 
    else {
      temp.tv_sec = ts_after.tv_sec - ts_before.tv_sec;
      temp.tv_nsec = ts_after.tv_nsec - ts_before.tv_nsec;
    }    
    // express delay in milliseconds (network delays are usually ms)
    time_diff = (double) temp.tv_sec * 1000.0 + (double) temp.tv_nsec / 1000000.0;
  }
  // free memory allocated for domain
  ldns_rdf_deep_free(domain);
  //debug std::cerr << "Query for " << domain_name << " " << time_diff << std::endl;
  return time_diff;
}

DnsResolver::~DnsResolver() {
  ldns_resolver_deep_free(resolver);
}



