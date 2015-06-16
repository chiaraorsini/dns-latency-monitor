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

#include "DnsDbHandler.hpp"
#include <math.h>



DnsDbHandler::DnsDbHandler(const char *db_name,
			   const char *server,
			   const char *user,
			   const char *password,
			   const char *socket,
			   unsigned int port) {
  try{
    // mysqlpp::Connection db_conn() - default ctor
    // exceptions are enabled by default
    // enabling automatic re-connection
    db_conn.set_option(new mysqlpp::ReconnectOption(true));
    // connecting to db
    db_conn.connect(db_name, server, user, password, port);  
    // set time zone UTC
    mysqlpp::Query query = db_conn.query("SET time_zone='+0:0'");
    // check exceptions + http://tangentsoft.net/mysql++/doc/html/userman/tutorial.html#examples
    mysqlpp::SimpleResult res = query.execute();
    if (!res) {
      throw std::string("Can't create DnsDbHandler() - Failed to set UTC time zone");
    }
    // check if tables exists, and in case create them
    std::stringstream s;
    s << "CREATE TABLE IF NOT EXISTS `top_domains` ( ";
    s << "`id` mediumint(9) NOT NULL AUTO_INCREMENT, ";
    s << "`rank` int(11) NOT NULL, ";
    s << "`domain` text NOT NULL, ";
    s << "PRIMARY KEY (`id`) ";
    s << ") ENGINE=InnoDB AUTO_INCREMENT=11 DEFAULT CHARSET=latin1; ";
    query = db_conn.query(s.str());
    res = query.execute();
    if (!res) {
      throw std::string("Can't create DnsDbHandler() - Failed to create top_domains table");
    }
    s.str("");
    // populate table
    s << "INSERT INTO `top_domains` VALUES (1,1,'google.com'),(2,2,'facebook.com'),(3,3,'youtube.com'),(4,4,'yahoo.com'),(5,5,'live.com'),(6,6,'wikipedia.org'),(7,7,'baidu.com'),(8,8,'blogger.com'),(9,9,'msn.com'),(10,10,'qq.com') ON DUPLICATE KEY UPDATE rank=rank, domain=domain";
    query = db_conn.query(s.str());
    res = query.execute();
    if (!res) {
      throw std::string("Can't create DnsDbHandler() - Failed to populate top_domains");
    }
    s.str("");
    s << "CREATE TABLE IF NOT EXISTS  `domain_stats` ( ";
    s << "`domain_id` mediumint(9) NOT NULL, ";
    s << "`latency_avg` float DEFAULT NULL, ";
    s << "`latency_stdev` float DEFAULT NULL, ";
    s << "`num_queries` int(11) NOT NULL, ";
    s << "`first_ts` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00', ";
    s << "`last_ts` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00', ";
    s << "PRIMARY KEY (`domain_id`), ";
    s << "CONSTRAINT `domain_stats_ibfk_1` FOREIGN KEY (`domain_id`) REFERENCES `top_domains` (`id`) ";
    s << ") ENGINE=InnoDB DEFAULT CHARSET=latin1; ";
    query = db_conn.query(s.str());
    res = query.execute();
    if (!res) {
      throw std::string("Can't create DnsDbHandler() - Failed to create domain_stats table");
    }
    #if defined(HAVE_PTHREAD_H) && HAVE_PTHREAD_H == 1
    pthread_mutex_init(&db_conn_mutex, NULL);
    #endif
  }
  catch(std::string s){
    throw s;
  }
  catch(std::exception& e) {
    std::stringstream es;
    es << "Can't create DnsDbHandler() -> " << e.what();
    throw es.str();
  }
  // default
  catch(...) {
    throw std::string("Can't create DnsDbHandler()");
  }
}


std::map<int,std::string> DnsDbHandler::get_top_n_domains(unsigned int n) {
  std::map<int,std::string> top_domains;
  std::stringstream s;
  s << "SELECT id,domain ";
  s << "FROM top_domains ";
  s << "ORDER BY rank ASC ";
  s << "LIMIT " << n;
  try{
    mysqlpp::Query query = db_conn.query(s.str());
    mysqlpp::UseQueryResult res = query.use();
    if (res) {
      // Get each row in result set, and print its contents
      while (mysqlpp::Row row = res.fetch_row()) {
	top_domains.insert(std::make_pair(row["id"] , row["domain"]));
      }
    }
  }
  catch(std::exception& e) {
    std::stringstream es;
    es << "Can't get_top_n_domains() -> " << e.what();
    throw es.str();
  }
  // default
  catch(...) {
    throw std::string("Can't get_top_n_domains()");
  }
  return top_domains;
}



void DnsDbHandler::update_dns_stats(int domain_id,
				    double latency,
				    int current_ts) {
  try {
    // check if there is already an entry for the current domain,
    // in case retrieve the statistics at the previous step
    // i.e. avg, variance, stdev at step n-1
    std::stringstream s;
    s << "SELECT latency_avg, latency_stdev, num_queries, UNIX_TIMESTAMP(first_ts) as unix_ts ";
    s << "FROM domain_stats ";
    s << "WHERE domain_id = " << domain_id;
#if defined(HAVE_PTHREAD_H) && HAVE_PTHREAD_H == 1
    // ask for db_connection resource
    pthread_mutex_lock(&db_conn_mutex);
    //debug std::cout << domain_id << " reads table start" << std::endl;
#endif
    mysqlpp::Query query = db_conn.query(s.str());
    mysqlpp::StoreQueryResult res = query.store();
#if defined(HAVE_PTHREAD_H) && HAVE_PTHREAD_H == 1
    //debug  std::cout << domain_id << " reads table stop" << std::endl;
    // release mutex on db_conn 
    pthread_mutex_unlock(&db_conn_mutex);
#endif
    double avg_latency_nminus1 = 0;
    double stdev_latency_nminus1 = 0;
    int nminus1 = 0;
    long int first_ts = 0;
    if (res) {  
      // domain_id is a primary key, query cannot 
      // retrieve more than one result
      if(res.num_rows() == 1) {
	avg_latency_nminus1 = res[0]["latency_avg"];
	stdev_latency_nminus1 = res[0]["latency_stdev"];
	nminus1 = res[0]["num_queries"];
	first_ts = res[0]["unix_ts"];
      }
      // else, the row is empty
    }
    else {
      std::stringstream es;
      es << "Failed to get domain_stats table: " << query.error() << std::endl;
      throw es.str();
    }
    // if it is the first entry, then current_ts is the first_ts
    if(first_ts == 0) {
      first_ts = current_ts;
    }
    // incremental computation of mean and stdev
    // Formula can be found here
    // http://math.stackexchange.com/questions/102978/incremental-computation-of-standard-deviation
    double var_latency_nminus1 = pow(stdev_latency_nminus1,2.0);
    int n = nminus1 + 1;
    double avg_latency_n = 0;   // avg, variance, stdev at step n
    double var_latency_n = 0; 
    double stdev_latency_n = 0;
    avg_latency_n = (double) (avg_latency_nminus1 * (n-1) + latency) / (double) n;
    var_latency_n = (n-1) * var_latency_nminus1;
    var_latency_n += pow((latency - avg_latency_n),2.0);
    var_latency_n += (n-1) * pow((avg_latency_nminus1 - avg_latency_n),2.0);
    var_latency_n = var_latency_n / (double) n;
    stdev_latency_n = sqrt(var_latency_n);
    // inserting new stats in database
    s.str(""); // cleaning stringstream  buffer
    s << "INSERT INTO domain_stats";
    s << "(domain_id, latency_avg, latency_stdev, num_queries, first_ts, last_ts) ";
    s << "VALUES(" << domain_id << ", ";
    s << avg_latency_n << ", " << stdev_latency_n << ", " << n << ", ";
    s << "FROM_UNIXTIME(" << first_ts << "), " << "FROM_UNIXTIME(" << current_ts << ") ) ";
    // if the entry already exists we do not have to update the domain_id and first_ts
    s << "ON DUPLICATE KEY UPDATE ";
    s << "latency_avg=VALUES(latency_avg), latency_stdev=VALUES(latency_stdev), ";
    s << "num_queries=VALUES(num_queries), last_ts=VALUES(last_ts)";
#if defined(HAVE_PTHREAD_H) && HAVE_PTHREAD_H == 1
    // ask for db_connection resource
    pthread_mutex_lock(&db_conn_mutex);
    //debug std::cout << domain_id << " writes table start" << std::endl;
#endif
    query = db_conn.query(s.str());
    query.exec();
#if defined(HAVE_PTHREAD_H) && HAVE_PTHREAD_H == 1
    //debug std::cout << domain_id << " writes table stop" << std::endl;
    // release mutex on db_conn 
    pthread_mutex_unlock(&db_conn_mutex);
#endif
  }
  // release mutex on db_conn if an exception is thrown
  catch(std::string ex_string) {
    std::cout << domain_id << " exception" << std::endl;    
#if defined(HAVE_PTHREAD_H) && HAVE_PTHREAD_H == 1
    pthread_mutex_unlock(&db_conn_mutex);
#endif
    std::stringstream es;
    es << "Can't update_dns_stats() -> " << ex_string;
    throw es.str();
  }
  catch(std::exception& e) {
    std::cout << domain_id << " exception" << std::endl;    
#if defined(HAVE_PTHREAD_H) && HAVE_PTHREAD_H == 1
    pthread_mutex_unlock(&db_conn_mutex);    
#endif
    std::stringstream es;
    es << "Can't update_dns_stats() -> " << e.what();
    throw es.str();
  }
  // default
  catch(...) {
    std::cout << domain_id << " exception" << std::endl;    
#if defined(HAVE_PTHREAD_H) && HAVE_PTHREAD_H == 1
    pthread_mutex_unlock(&db_conn_mutex);
#endif
    throw std::string("Can't update_dns_stats()");    
  }
}


DnsDbHandler::~DnsDbHandler() {
  db_conn.disconnect();
}

