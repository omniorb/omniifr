//                            Package   : omniIFR
// daemon_unix.h              Created   : 2004/06/26
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

#ifndef OMNIIFR__DAEMON_UNIX_H
#define OMNIIFR__DAEMON_UNIX_H

#ifdef __WIN32__
#  error("This file is not intended for Windows.")
#endif

namespace Omniifr {

/** Utility class that contains various methods for running omniIFR as a
 * Unix daemon. Features: pidfile support, forking, redirect omniORB trace
 * to syslog (or tracefile).
 */
class DaemonImpl
{
public:
  static DaemonImpl _inst;

  char* _tracefile;   ///< The tracefile name (if any).
  bool  _foreground;  ///< TRUE for debug mode (run in foreground)
  char* _pidfile;     ///< The pidfile name (if any).
  int   _pipe[2];     ///< Unnamed pipe for child->parent comms.
  bool  _havePidfile; ///< Is there a pidfile for us to clean up?
  bool  _haveParent;  ///< Is there a parent for us to clean up?
  bool  _haveSyslog;  ///< Should we close syslog before quitting?

  DaemonImpl();
  ~DaemonImpl();

  void tracefile(const char* val); ///< Set _tracefile.
  void pidfile(const char* val);   ///< Set _pidfile.
  void foreground(bool val);       ///< Set _foreground.

  /** Does nothing on Unix. */
  void initialize(int&,char**&);

  /** Puts the current process into the background. Redirects the omniORB log
   * output to syslog (or 'tracefile', if it is set).
   */
  void daemonize();

  /** Called to signal that all startup operations have completed OK.
   * Notifies the parent process and redirects stdout & stderr to 'tracefile'
   * (or else /dev/null).
   */
  void runningOk();

  /** Exit handler called (indirectly) by ::on_exit() - shuts down the daemon.
   * Deletes pidfile (if we have one), notifies the parent (if we have one).
   */
  void shutdown(int status);

  /** Callback, used as a parameter to omniORB::setLogFunction(). */
  static void log(const char* message);

private:
  /** Performs the actual fork. */
  void fork();

  /** Redirect stdout & stderr to filename.
   * Also redirects stdin from /dev/null
   */
  void redirectStreamsTo(const char* filename);

  /** Opens a (new?) file called 'filename' for writing, and uses it to
   * hijack stream 'fd'.
   */
  int openFileFor(int fd, const char* filename, int flags);

  /** If pidfile exists & contains a running process then shutdown() (Unix).
   * Also shuts down if pidfile is inaccessible.
   */
  void checkPidfileOrShutdown();

  void writePidfile();

  /** Called by the parent process (Unix). Waits for the child to return an
   * exit status. The status is usually '0' - indicating that the daemon has
   * started successfully.
   */
  int waitForChild();
  
  /** Tells the parent to exit with the given status (Unix). */
  void notifyParent(int status);
};

} // end namespace Omniifr

#endif // OMNIIFR__DAEMON_UNIX_H
