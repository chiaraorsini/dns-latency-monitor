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


#include <iostream>
#include <thread>
#include <map>

#include <mysql++.h>
#include <ldns/ldns.h>
#include "dns_latency_monitor-config.h"

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
     
#include "RecurrentDnsStatsMonitor.hpp"


/* Flag set by ‘--verbose’. */
static int help_flag;

static int usage() {
  std::cout << "NAME:" << std::endl;
  std::cout << "\t" << "dns-latency-monitor - store dns latency information in a mysql database " << std::endl;
  std::cout << std::endl;
  std::cout << "SYNOPSIS:" << std::endl;
  std::cout << "\t" << "dns-latency-monitor\t --database mysql_database " << std::endl;
  std::cout << "\t" << "\t\t\t" << " [--frequency query_frequency] " << std::endl;
  std::cout << "\t" << "\t\t\t" << " [--cycles max_cycles] " << std::endl;
  std::cout << "\t" << "\t\t\t" << " [--user mysql_user] " << std::endl;
  std::cout << "\t" << "\t\t\t" << " [--password mysql_password] " << std::endl;
  std::cout << "\t" << "\t\t\t" << " [--machine mysql_server_ip] " << std::endl;
  std::cout << "\t" << "\t\t\t" << " [--socket mysql_socket] " << std::endl;
  std::cout << "\t" << "\t\t\t" << " [--port mysql_port] " << std::endl;
  std::cout << std::endl;
  std::cout << "OPTIONS:" << std::endl;
  std::cout << "\t" << "database - mysql database name (mandatory)" << std::endl;
  std::cout << "\t" << "user - username to access the mysql database " << std::endl;
  std::cout << "\t" << "password - password to access the mysql database " << std::endl;
  std::cout << "\t" << "machine - IP address of the mysql database " << std::endl;
  std::cout << "\t" << "socket - socket used to access the mysql database " << std::endl;
  std::cout << "\t" << "port - port used to access the mysql database " << std::endl;
  std::cout << "\t" << "frequency - DNS query frequency in seconds (default 60 s)" << std::endl;
  std::cout << "\t" << "cycles - maximum number of iterations (default 0, i.e. infinite process)" << std::endl;

  std::cout << std::endl;

  return 0;
}


int main(int argc, char * argv[]) {

  unsigned int frequency = 60;
  unsigned int cycles = 0;
  char * db_name = NULL;
  char * server = NULL;
  char * user = NULL;
  char * password = NULL;
  char * socket = NULL;
  unsigned int port = 0;
  int c;

  struct option long_options[] =  {
    /* These options set a flag. */
    {"help", no_argument, &help_flag, 1},
    /* These options don't set a flag. */
    {"frequency", required_argument, 0, 'f'},
    {"database",  required_argument, 0, 'd'},
    {"user",      required_argument, 0, 'u'},
    {"password",  required_argument, 0, 'p'},
    {"machine",   required_argument, 0, 'm'},
    {"socket",    required_argument, 0, 's'},
    {"port",      required_argument, 0, 'o'},
    {"cycles",    required_argument, 0, 'c'},
    // Terminate the array with an element containing all zero
      {0, 0, 0, 0}
    };

  int option_index = 0;    
  while((c = getopt_long (argc, argv, "f:d:u:p:m:s:p:",
			  long_options, &option_index)) != -1) {     
    switch (c){
    case 'f':
      frequency = atoi(optarg);     
      break;     
    case 'c':
      cycles = atoi(optarg);     
      break;     
    case 'd':
      db_name = strdup(optarg);
      break;     
    case 'm':
      server = strdup(optarg);
      break;     
    case 'u':
      user = strdup(optarg);
      break;     
    case 'p':
      password = strdup(optarg);
      break;     
    case 's':
      socket = strdup(optarg);
      break;     
    case 'o':
      port = atoi(optarg);     
      break;     
    case '?':
    default:
      /* getopt_long already printed an error message. */
      // unrecognized option
      return usage();
    }      
    if(help_flag) {
      return usage();
    }     
  }

  if(db_name == NULL) {
    std::cout << "database name is a mandatory option" << std::endl;
    return usage();
  }
  try{
    RecurrentDnsStatsMonitor rdsm(db_name, server, user, password, socket, port);
#if defined(HAVE_PTHREAD_H) && HAVE_PTHREAD_H == 1
    rdsm.parallel_run(frequency,cycles);
#else
    rdsm.run(frequency,cycles);
#endif
  } 
  catch(std::string s) {
    std::cerr << s << std::endl;
  }

  // cleaning up str parameters
  if(db_name != NULL) { free(db_name); }
  if(server != NULL) { free(server); }
  if(user != NULL) { free(user); }
  if(password != NULL) { free(password); }
  if(socket != NULL) { free(socket); }

  return 0;
}
