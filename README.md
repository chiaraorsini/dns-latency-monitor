# dns-latency-monitor
Tracking DNS performance to top sites

A c++ program on Linux/BSD(Mac) that periodically sends DNS queries to the nameservers of the top 10 Alexa domains and stores the latency values in a mysql table. The frequency of queries should be specified by the user on command line. The program  doesn't hit the DNS cache while trying to query for a site (it uses a random string prepended to a domain. E.g. to query foo.bar, it prepends a random string, e.g. 1234xpto.foo.bar.)

The code keeps track in db stats per domain about:
* the average query times
* standard deviation of DNS query times
* number of queries made so far
* time stamp of first query made per domain and last query made

Top 10 domains to query: 
1. google.com
2. facebook.com
3. youtube.com
4. yahoo.com
5. live.com
6. wikipedia.org
7. baidu.com
8. blogger.com
9. msn.com
10. qq.com

    

    
## Installation


$ autoreconf -vfi   # development version only
$ ./configure       # see comments below to setup the right configuration
$ make
$ (make install)    # optional


if ldns or mysqlpp are not installed in a folder in PATH
then provide the following options to configure:

 CPPFLAGS="-I/path_to_includes"
 LDFLAGS="-I/path_to_libs"

### Requirements 
a. Mysql lib, use mysql++:
 * http://tangentsoft.net/mysql++/
b. DNS lib, use ldns:
 * http://www.nlnetlabs.nl/projects/ldns/



