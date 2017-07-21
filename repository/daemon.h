//                            Package   : omniIFR
// service.h                  Created   : 2004/07/25
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

#ifndef OMNIIFR__SERVICE_H
#define OMNIIFR__SERVICE_H

namespace Omniifr {

/** Interface class that contains various methods for running omniIFR as a
 * background task.
 */
class Daemon
{
  Daemon(); ///< No implementation

public:
  Daemon(int& argc,char**& argv);
  virtual ~Daemon();

  void tracefile(const char* val); ///< Set _tracefile.
  void pidfile(const char* val);   ///< Set _pidfile.
  void foreground(bool val);       ///< Set _foreground.

  /** Redirects output streams to tracefile. */
  void daemonize();

  /** Called to signal that all startup operations have completed OK. */
  void runningOk();

}; // class Daemon

} // end namespace Omniifr

#endif // OMNIIFR__DAEMON_H

