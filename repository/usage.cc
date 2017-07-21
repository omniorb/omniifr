//                            Package   : omniIFR
//  usage.cc                  Created   : 2004/04/05
//                            Author    : Alex Tingle
//
//    Copyright (C) 2004 Alex Tingle.
//
//    This file is part of the omniIFR application.
//
//    omniIFR is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Lesser General Public
//    License as published by the Free Software Foundation; either
//    version 2.1 of the License, or (at your option) any later version.
//
//    omniIFR is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public
//    License along with this library; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#define NEED_PACKAGE_INFO

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifdef HAVE_IOSTREAM
#  include <iostream>
#else
#  include <iostream.h>
#endif

#ifdef HAVE_STDLIB_H
#  include <stdlib.h> // exit()
#endif

#ifdef HAVE_STD_IOSTREAM
using namespace std;
#endif

void usage(int argc, char **argv)
{
  cerr<<
  "\nRun the omniIFR server.\n"
  "syntax: "<<(argc?argv[0]:"omniIFR")
            <<" OPTIONS -ORBendPoint giop:tcp::11173\n"
  "OPTIONS:\n"
#ifndef __WIN32__
  " -f           Stay in the foreground.\n"
  " -l PATH      full path to data directory [/var/lib/omniifr]\n"
  " -P PIDFILE   keep track of running instance in PIDFILE.\n"
#endif
  " -r           read only mode.\n"
  " -t FILE      Send trace messages to FILE instead of syslog.\n"
  " -V           display version\n"
  " -h           display this help text\n" << endl;
  exit(-1);
}


void version()
{
  cerr<<PACKAGE_STRING<<"\nCopyright (C) 2004-2005 Alex Tingle"<<endl;
  exit(-1);
}
