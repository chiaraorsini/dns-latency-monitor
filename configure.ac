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

AC_PREREQ([2.68])

AC_INIT([dns-latency-monitor], [0.0.1], [chiara@caida.org])
AC_CONFIG_AUX_DIR(config)

DNS_LATENCY_MONITOR_MAJOR_VERSION=0
DNS_LATENCY_MONITOR_MID_VERSION=0
DNS_LATENCY_MONITOR_MINOR_VERSION=0

AC_DEFINE_UNQUOTED([DNS_LATENCY_MONITOR_MAJOR_VERSION],$DNS_LATENCY_MONITOR_MAJOR_VERSION,
	[dns-latency-monitor major version])
AC_DEFINE_UNQUOTED([DNS_LATENCY_MONITOR_MID_VERSION],$DNS_LATENCY_MONITOR_MID_VERSION,
	[dns-latency-monitor mid version])
AC_DEFINE_UNQUOTED([DNS_LATENCY_MONITOR_MINOR_VERSION],$DNS_LATENCY_MONITOR_MINOR_VERSION,
	[dns-latency-monitor minor version])


# check if the source folder is available
AC_CONFIG_SRCDIR([src/dns_latency_monitor.cpp])
AC_CONFIG_HEADERS([src/dns_latency_monitor-config.h])


# check for C++ preprocessor and compiler
AC_PROG_CXXCPP
AC_PROG_CXX

# automake initialization
AM_INIT_AUTOMAKE([1.9])


# use the C compiler for the following checks
AC_LANG([C])

AC_CHECK_HEADERS([mysql.h ldns/ldns.h pthread.h])

# check mysqlclient_r c library (reentrant version -> thread safe)
AC_CHECK_LIB([mysqlclient_r], [mysql_query], ,
			    [AC_MSG_ERROR( [libmysqlclient_r required ])])

# check ldns c library
AC_CHECK_LIB([ldns], [ldns_resolver_query], ,
			    [AC_MSG_ERROR( [libldns required ])])

# check pthread library
AC_CHECK_LIB([pthread], pthread_create, [PTHREAD_LIBS+=-lpthread], [AC_MSG_NOTICE( [pthread not found])])

# use the C++ compiler for the following checks
AC_LANG([C++])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([string])
AC_CHECK_HEADERS([iostream])



##########################################################################
# check for mysqlpp library
##########################################################################

# store current *FLAGS and merge with AM_*FLAGS for compilation and linker check   
OLD_CXXFLAGS=$CXXFLAGS;
OLD_LDFLAGS=$LDFLAGS;
CXXFLAGS="$AM_CXXFLAGS $CXXFLAGS"
LDFLAGS="$AM_LDFLAGS $LDFLAGS"

# ensure the library to check for is covered by the LIBS variable
OLD_LIBS=$LIBS
LIBS="$LIBS -lmysqlpp"


# check for mysqlpp library headers   
AC_MSG_CHECKING([for the mysqlpp library headers])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <mysql++.h>]])],
                  [# actions to do if mysqlpp headers are found      
		   AC_MSG_RESULT([found])
		   # try to link the function 'mysqlpp::Connections' out of library mysqlpp	
		   AC_MSG_CHECKING([whether the mysqlpp library can be linked])
		   AC_LINK_IFELSE(	   
		   		   [AC_LANG_PROGRAM( [[#include <mysql++.h>]],
				                     [[mysqlpp::Connection c(false);]]
				                    )],
		                   [# actions to do if mysqlpp is linked correctly
		   		    AC_MSG_RESULT([yes])
				    LIBS=$OLD_LIBS; # reset to old value 
				    FOUND_mysqlpp=1;
				   ],
		                   [# actions to do if mysqlpp is not linked correctly
		   		    AC_MSG_RESULT([no])
				    FOUND_mysqlpp=0;
				   ]
		   )		   
		  ], 
                  [# actions to do if mysqlpp headers are not found      
		   AC_MSG_RESULT([not found])
		   FOUND_mysqlpp=0;
		  ] 
                 ) 


# reset original *FLAGS
CXXFLAGS=$OLD_CXXFLAGS
LDFLAGS=$OLD_LDFLAGS

# handle check results
if test $FOUND_mysqlpp != 1; then
    AC_MSG_NOTICE([])
    AC_MSG_NOTICE([The mysqlpp library was not found!])
    if test  $mysqlppPATHSET = 1 ; then
        AC_MSG_NOTICE([ The install prefix '$with_mysqlpp' for the mysqlpp library was set.])
        AC_MSG_NOTICE([ --> Maybe wrong ???])
    else
        AC_MSG_NOTICE([ No non-standard install prefix was set.])
        AC_MSG_NOTICE([ --> You might want to use '--with-mysqlpp=PREFIX' ?!?])
    fi
    AC_MSG_NOTICE([])
    AC_MSG_ERROR([ mysqlpp library is an essential dependency : cannot build and stop here !])
fi

##########################################################################


# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T



# files to generate via autotools (prepare .am or .in source files)
AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([src/Makefile])

# finally this generates the Makefiles etc. for the build
AC_OUTPUT



###############################################################



dnl AC_CONFIG_MACRO_DIR([m4])

dnl AC_CONFIG_SRCDIR([src/test.cpp])
dnl AC_CONFIG_HEADERS([config.h])

dnl # Checks for programs.
dnl AC_PROG_LIBTOOL
dnl AC_PROG_CXX
dnl AM_PROG_LIBTOOL
dnl AC_LIBTOOL_CXX

dnl AC_SYS_LARGEFILE

dnl AH_VERBATIM([_GNU_SOURCE],
dnl [/* Enable GNU extensions on systems that have them.  */
dnl #ifndef _GNU_SOURCE
dnl #define _GNU_SOURCE
dnl #endif])

dnl AC_CHECK_FUNCS([gettimeofday memset strdup strstr strsep strlcpy vasprintf])

dnl # should we dump debug output to stderr and not optmize the build?

dnl AC_MSG_CHECKING([whether to build with debug information])
dnl AC_ARG_ENABLE([debug],
dnl     [AS_HELP_STRING([--enable-debug],
dnl         [enable debug data generation (def=no)])],
dnl     [debugit="$enableval"],
dnl     [debugit=no])
dnl AC_MSG_RESULT([$debugit])

dnl if test x"$debugit" = x"yes"; then
dnl     AC_DEFINE([DEBUG],[],[Debug Mode])
dnl else
dnl     AC_DEFINE([NDEBUG],[],[No-debug Mode])
dnl fi

dnl # Checks for typedefs, structures, and compiler characteristics.
dnl AC_C_INLINE
dnl AC_TYPE_SIZE_T
dnl AC_TYPE_UINT16_T
dnl AC_TYPE_UINT32_T
dnl AC_TYPE_UINT64_T
dnl AC_TYPE_UINT8_T

dnl # Checks for library functions.
dnl AC_FUNC_MALLOC
dnl AC_FUNC_REALLOC


dnl AC_CHECK_LIB([mysqlclient], [mysql_query], ,
dnl 			    [AC_MSG_ERROR( [libmysqlclient required ])])

dnl dnl # we use libbgpstream for reading a stream of records
dnl dnl AC_CHECK_LIB([bgpstream], [bgpstream_get_next_record], ,
dnl dnl                [AC_MSG_ERROR( [libbgpstream required ])])

dnl # Checks for header files.
dnl AC_CHECK_HEADERS([arpa/inet.h inttypes.h limits.h math.h stdlib.h string.h \
dnl 			      time.h sys/time.h])



dnl # we may want to come back later and add compile-time configuration for things
dnl # like datastructure providers, but for now it will all get compiled


dnl AC_SUBST([DNS_LATENCY_MONITOR_MAJOR_VERSION])
dnl AC_SUBST([DNS_LATENCY_MONITOR_MID_VERSION])
dnl AC_SUBST([DNS_LATENCY_MONITOR_MINOR_VERSION])

dnl AC_HEADER_ASSERT

dnl AC_CONFIG_FILES([Makefile
dnl 		src/Makefile
dnl 		])
dnl AC_OUTPUT

