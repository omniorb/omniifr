//                            Package   : omniIFR
// daemon_windows.h           Created   : 2004/07/30
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

#ifndef OMNIIFR__DAEMON_WINDOWS_H
#define OMNIIFR__DAEMON_WINDOWS_H

#ifndef __WIN32__
#  error("This file is only intended for Windows.")
#endif

#include <windows.h>
#include <winsvc.h>
#include <iostream>

namespace Omniifr {

/** Singleton class that contains various methods for running a
 * Windows service.
 */
class Service
{
public:
  Service();
  ~Service();

  void start(int& argc,char**& argv);

  void tracefile(const char* val); ///< Set _tracefile.
  void pidfile(const char* val);   ///< Set _pidfile.
  void foreground(bool val);       ///< Set _foreground.

  /** Redirects output streams to tracefile. */
  void daemonize();

  /** Called to signal that all startup operations have completed OK. */
  void runningOk();

  /** Exit handler set with ::on_exit() - shuts down the service. */
  void shutdown();

  // Callbacks.

  /** Callback, used as a parameter to omniORB::setLogFunction(). */
  static void log(const char* message);

  /** Handles control codes from the Service Control Manager. */
  static void ctrlHandler(DWORD controlCode);

private:
  char*         _tracefile;   ///< The tracefile name (if any).
  const char*   _regSubKey;
  bool          _serviceRunning;
  int           _callCount;
  char*         _parameters; ///< Stores parameters read from the registry.
  char**        _argv; ///< Replacement argv array, read from registry.
  std::ostream* _logstream;
  SERVICE_STATUS_HANDLE _serviceStatusHandle; ///< Windows thing

  void Service::setArgcArgv(int& argc,char**& argv);
  void install(int argc,char** argv) const;
  void uninstall() const;

  /** Populates _parameters from the Registry. */
  void readParameters();
  /** Writes args 2+ to the Registry. */
  void writeParameters(int argc, char** argv) const;

  bool Service::setServiceStatus(
    DWORD currentState,
    DWORD win32ExitCode,
    DWORD serviceSpecificExitCode,
    DWORD checkPoint,
    DWORD waitHint
  );

}; // class Service

} // end namespace Omniifr

#endif // OMNIIFR__DAEMON_WINDOWS_H

