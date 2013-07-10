/* Copyright (C) 2003 MySQL AB & Jorge del Conde

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.
    
  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
  MA 02111-1307, USA 
*/
#include "mysqlcc_main.h"
#include <my_global.h>
#include <my_sys.h>
#ifdef VERSION
#undef VERSION
#endif
#include <m_string.h>
#include <my_getopt.h>
#ifndef NO_QT
#define NO_QT
#endif
#include "config.h"

#ifdef WIN32
#if ((defined(__BORLANDC__) || defined(__TURBOC__)) && \
!defined(__CONSOLE__)) || defined(__WATCOMC__)
#define main qMain
#endif
#endif

static char *progname;
static void usage();
static void print_version();

static const char *load_default_groups[]= { "mysqlcc",0 };

static ulong select_limit,my_net_buffer_length,max_join_size,my_max_allowed_packet,opt_connect_timeout=0;
static uint opt_local_infile=0;

static struct my_option my_long_options[] =
{
  {"compress", 'C', "Use compression in server/client protocol.", 0, 0, 0, GET_NO_ARG, NO_ARG, 0, 0, 0, 0, 0, 0},

  {"blocking_queries", 'b', "Use blocking queries.", 0, 0, 0, GET_NO_ARG, NO_ARG, 0, 0, 0, 0, 0, 0},

  {"syntax", 'y', "Enable syntax highlighting and completion.", 0, 0, 0, GET_NO_ARG, NO_ARG, 0, 0, 0, 0, 0, 0},

  {"database", 'd', "Database to use.", 0, 0, 0, GET_STR, REQUIRED_ARG, 0, 0, 0, 0, 0, 0},

  {"host", 'h', "Connect to host", 0, 0, 0, GET_STR, REQUIRED_ARG, 0, 0, 0, 0, 0, 0},

  {"user", 'u', "User for login if not current user.", 0, 0, 0, GET_STR, REQUIRED_ARG, 0, 0, 0, 0, 0, 0},

  {"password", 'p',
   "Password to use when connecting to server. If password is not given, MySQL Control Center will prompt for the password.",
   0, 0, 0, GET_STR, OPT_ARG, 0, 0, 0, 0, 0, 0},

  {"port", 'P', "Port number to use for connection.", 0, 0, 0, GET_UINT, REQUIRED_ARG, 0, 0, 0, 0, 0, 0},

#ifndef WIN32
  {"socket", 'S', "Socket file to use for connection.", 0, 0, 0, GET_STR, REQUIRED_ARG, 0, 0, 0, 0, 0, 0},
#endif

  {"connection_name", 'c', "This is a synonym for --server.", 0, 0, 0, GET_STR, REQUIRED_ARG, 0, 0, 0, 0, 0, 0},

  {"server", 's', "MySQL Control Center connection name.", 0, 0, 0, GET_STR, REQUIRED_ARG, 0, 0, 0, 0, 0, 0},

  {"query", 'q', "Open a query window on startup.", 0, 0, 0, GET_NO_ARG, NO_ARG, 0, 0, 0, 0, 0, 0},

  {"register", 'r', "Open the 'Register Server' dialog on startup.", 0, 0, 0, GET_NO_ARG, NO_ARG, 0, 0, 0, 0, 0, 0},
  
  {"help", '?', "Display this help and exit.", 0, 0, 0, GET_NO_ARG, NO_ARG, 0,  0, 0, 0, 0, 0},
  
  {"syntax_file", 'Y', "Syntax file for completion.", 0, 0, 0, GET_STR, REQUIRED_ARG, 0, 0, 0, 0, 0, 0},

  {"history_size", 'H', "History size for the query window.", 0, 0, 0, GET_UINT, REQUIRED_ARG, 0, 0, 0, 0, 0, 0},

  {"translations_path", 'T', "Path to the directory where MySQL Control Center translations are located.",
   0, 0, 0, GET_STR, REQUIRED_ARG, 0, 0, 0, 0, 0, 0},  

  {"plugins_path", 'g', "Path to the directory where MySQL Control Center plugins are located.",
   0, 0, 0, GET_STR, REQUIRED_ARG, 0, 0, 0, 0, 0, 0},  

  {"version", 'V', "Print version information and exit.", 0, 0, 0, GET_NO_ARG, NO_ARG, 0, 0, 0, 0, 0, 0},

  {"connect_timeout", OPT_CONNECT_TIMEOUT, "", (uchar**) &opt_connect_timeout,
   (uchar**) &opt_connect_timeout, 0, GET_ULONG, REQUIRED_ARG, 0, 0, 3600*12, 0,
   0, (void*)1},

  {"select_limit", OPT_SELECT_LIMIT, "", (uchar**) &select_limit,
   (uchar**) &select_limit, 0, GET_ULONG, REQUIRED_ARG, 1000L, 1, ~0L, 0, 1, 0},
  
  {"net_buffer_length", OPT_NET_BUFFER_LENGTH, "",
   (uchar**) &my_net_buffer_length, (uchar**) &my_net_buffer_length, 0, GET_ULONG,
   REQUIRED_ARG, 16384, 1024, 512*1024*1024L, 0, 1024, 0},

  {"max_allowed_packet", OPT_MAX_ALLOWED_PACKET, "",
   (uchar**) &my_max_allowed_packet, (uchar**) &my_max_allowed_packet, 0, GET_ULONG,
   REQUIRED_ARG, 16 *1024L*1024L, 4096, 512*1024L*1024L, 0,
   1024, 0},

  {"local-infile", OPT_LOCAL_INFILE, "Enable/disable LOAD DATA LOCAL INFILE.",
   (uchar**) &opt_local_infile,
   (uchar**) &opt_local_infile, 0, GET_BOOL, OPT_ARG, 0, 0, 0, 0, 0, 0},

  {"max_join_size", OPT_MAX_JOIN_SIZE, "", (uchar**) &max_join_size,
   (uchar**) &max_join_size, 0, GET_ULONG, REQUIRED_ARG, 1000000L, 1, ~0L, 0, 1,
   0},

  { 0, 0, 0, 0, 0, 0, GET_NO_ARG, NO_ARG, 0, 0, 0, 0, 0, 0}
};


static my_bool
get_one_option(int optid, const struct my_option * opt, char *argument)
{
  switch(optid) {
  case 'V':
      print_version();
      exit(-1);

  case '?':
    usage();

  default:
    {
      int id;
      switch (optid) {
        case OPT_CONNECT_TIMEOUT: id = 't';
          break;

        case OPT_MAX_ALLOWED_PACKET: id = 'A';
          break;

        case OPT_NET_BUFFER_LENGTH: id = 'N';
          break;

        case OPT_SELECT_LIMIT: id = 'l';
          break;

        case OPT_MAX_JOIN_SIZE: id = 'J';
          break;

        case OPT_LOCAL_INFILE: id = 'L';
          break;

        default: id = optid;
          break;
      }
      if (opt->var_type == GET_NO_ARG && opt->arg_type == NO_ARG)  //Boolean options
        add_argument(id, "");
      else
        add_argument(id, argument);
    }
    break;
  }
  return 0;
}


static void print_version()
{
  printf("%s version %s%s by Jorge del Conde\n", progname, VERSION, BRANCH);
}


static void usage()
{
  print_version();
  printf("usage: %s [options] [database]\n\n", progname);
  printf("Options:\n");
  my_print_help(my_long_options);

#ifndef WIN32  //Win32 doesn't support load_defaults yet.
  print_defaults("my", load_default_groups);  
#endif

  my_print_variables(my_long_options);
  exit(-1);
}


int main(int argc, char *argv[])
{
  int pid = 0;
  int t = 0;  
  int ret = -1;
  char **save_argv;
  load_defaults("my",load_default_groups,&argc,&argv);
  save_argv = argv;
  progname= argv[0];
  if (handle_options(&argc, &argv, my_long_options, get_one_option))
    exit(-1);
  if (!argv[0] || !argv[1] || (pid = atoi(argv[0])) <= 0 || (t= atoi(argv[1])) <= 0)  
    ret = mysqlcc_main(argc, argv);
  free_defaults(save_argv);
  return ret;
}
