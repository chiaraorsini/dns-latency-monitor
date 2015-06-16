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

#include "RecurrentDnsStatsMonitor.hpp"
#include <ctime>
#include <exception>
#include <vector>

#if defined(HAVE_PTHREAD_H) && HAVE_PTHREAD_H == 1
#include <pthread.h>
#endif


RecurrentDnsStatsMonitor::RecurrentDnsStatsMonitor(const char * db_name,
						   const char * server,
						   const char * user,
						   const char * password,
						   const char * socket,
						   unsigned int port) 
  try : ddh(db_name, server, user, password, socket, port), dr() {
  // get top 10 domains from database
  top_domains = ddh.get_top_n_domains(10);
  dns_test_frequency = 60; // default 
  max_num_cycles = 0; // default    
  // database handler and dns resolve are constructed in the initialization list
}
catch(std::string s){
  throw std::string("Error in RecurrentDnsStatsMonitor() -> ") + s;
}

// this function is not visible outside this code unit
static std::string gen_random_string(const int len) {
  std::stringstream s;
  static const char alphanum[] =
    "0123456789"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz";
  s << "a";
  for (int i = 0; i < len; ++i) {
    s << alphanum[rand() % (sizeof(alphanum) - 1)];
  }
  return s.str();
}


void RecurrentDnsStatsMonitor::run(unsigned int frequency, unsigned int cycles) {
  dns_test_frequency = frequency;
  max_num_cycles = cycles;
  if(dns_test_frequency <= 0) { // minimum frequency is 1 second
    return;
  }
  bool check_cycles = false; // default: no check on number of cycles executed
  if(max_num_cycles > 0) {
    check_cycles = true;
  }
  std::string random_prepending;
  std::stringstream domain_to_query;
  double latency;
  while(true) {
    // we generate one random string to prepend per cycle
    random_prepending = gen_random_string(10);
    std::time_t cur_time = std::time(NULL);
    std::map<int,std::string>::const_iterator it;
    for(it = top_domains.begin(); it != top_domains.end(); it++) {
      domain_to_query.str("");
      domain_to_query << random_prepending << "." << it->second; // domain_name      
      latency = dr.query_nameserver(domain_to_query.str());
      ddh.update_dns_stats(it->first /*domain_id*/, latency, cur_time);	
    }    
    if(check_cycles) {
      max_num_cycles--;
      if(max_num_cycles == 0){
	break;
      }
    }
    // wait for <frequency> seconds
    sleep(dns_test_frequency);
  }
}


#if defined(HAVE_PTHREAD_H) && HAVE_PTHREAD_H == 1

void RecurrentDnsStatsMonitor::single_domain_run(int domain_id, std::string domain_name) {
  if(dns_test_frequency <= 0) { // minimum frequency is 1 second
    return;
  }
  // we do not modify the class variable (it is shared)
  int cycles = max_num_cycles;
  bool check_cycles = false; // default: no check on number of cycles executed
  if(cycles > 0) {
    check_cycles = true;
  }
  std::string random_prepending;
  std::stringstream domain_to_query;
  double latency;
  while(true) {
    // we generate one random string to prepend per cycle
    random_prepending = gen_random_string(10);
    std::time_t cur_time = std::time(NULL);
    domain_to_query << random_prepending << "." << domain_name; 
    latency = dr.query_nameserver(domain_to_query.str());
    ddh.update_dns_stats(domain_id, latency, cur_time);	    
    if(check_cycles) {
      cycles--;
      if(cycles == 0){
	break;
      }
    }
    // wait for <frequency> seconds
    sleep(dns_test_frequency);
    domain_to_query.str("");
  }
}

struct thread_parameters {
  int domain_id;
  std::string domain_name;  
  RecurrentDnsStatsMonitor * monitor;
};

// this function is not visible outside this code unit
static void * thread_run_wrapper(void * arg){
  try { 
    struct thread_parameters * par = (struct thread_parameters *) arg;
    par->monitor->single_domain_run(par->domain_id, par->domain_name);
  } 
  catch (...) { 
    std::cerr << "Error in thread run wrapper" << std::endl;
    pthread_exit(NULL);
  }
  pthread_exit(NULL);
}


void RecurrentDnsStatsMonitor::parallel_run(unsigned int frequency, unsigned int cycles) {
  try {
  // initialize common parameter
  dns_test_frequency = frequency;
  max_num_cycles = cycles;
  // create a thread for each domain
  int num_domains = top_domains.size();
  std::map<int,std::string>:: const_iterator d_it; // top_domains iterator
  // initialize thread parameters for each domain
  std::map<int,thread_parameters> domains_parameters;
  std::map<int,thread_parameters>:: iterator p_it; // domains_parameters iterator
  thread_parameters tp;
  int i = 0;
  for (d_it = top_domains.begin(), i=0; d_it != top_domains.end(); d_it++,i++) {
    tp.domain_id = d_it->first;
    tp.domain_name = d_it->second;
    tp.monitor = this;
    domains_parameters.insert(std::make_pair(i, tp));
  }
  // create a pthread_t structure for each domain
  std::vector<pthread_t> threads(num_domains);
  std::vector<pthread_t>::iterator t_it; // threads vector iterator
  std::vector<int> pthread_error_vector(num_domains);
  std::vector<int> :: iterator v;
  int rc;
  // launching threads
  for (i=0, p_it = domains_parameters.begin(); i < num_domains; i++, p_it++) {
    // int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg);
    rc = pthread_create(&(threads[i]), NULL /*default attr*/, thread_run_wrapper, &(p_it->second));
    pthread_error_vector[i] = (rc);
    if(rc){
      std::cerr << "Can't create thread: " << strerror(rc) << std::endl;
    }
  }
  // waiting for all threads to finish
  for (i=0; i < num_domains; i++) {
    if(pthread_error_vector[i] != 0) {
      continue; // do not join pthreads that failed to create
    }
    if(pthread_join(threads[i], NULL) != 0) {
      std::cerr << "Error joining thread" << std::endl;
    }
  }
  pthread_exit(NULL);  
  }
  catch (std::string s) { 
    throw std::string("Error in parallel_run() -> ") + s;
  }
  catch (...) { 
    throw "Error in parallel_run()";
  }
}
#endif

RecurrentDnsStatsMonitor::~RecurrentDnsStatsMonitor() {
  // internal object destructors are automatically called
}
