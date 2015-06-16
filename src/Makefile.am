#
# dns-latency-monitor
#
# Chiara Orsini
# chiara@caida.org
#
# This file is part of dns-latency-monitor.
#
# dns-latency-monitor is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# dns-latency-monitor is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with dns-latency-monitor.  If not, see <http://www.gnu.org/licenses/>.
#

AM_CPPFLAGS = -I$(top_srcdir) 

bin_PROGRAMS =  dns-latency-monitor

dns_latency_monitor_SOURCES = dns_latency_monitor.cpp       \
			      RecurrentDnsStatsMonitor.hpp  \
			      RecurrentDnsStatsMonitor.cpp  \
			      DnsResolver.hpp               \
			      DnsResolver.cpp               \
			      DnsDbHandler.hpp              \
			      DnsDbHandler.cpp 

dns_latency_monitor_LDADD = -lldns -lmysqlclient_r -lmysqlpp 

ACLOCAL_AMFLAGS = -I m4

CLEANFILES = *~
