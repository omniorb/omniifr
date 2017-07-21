//                            Package   : omniIFR
// daemon_unix.h              Created   : 2004/06/29
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

#include "daemon.h"
#include "main.h"
#include "daemon_unix.h"

#define NEED_PACKAGE_INFO
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifdef HAVE_IOSTREAM
#  include <iostream>
#  include <fstream>
#else
#  include <iostream.h>
#  include <fstream.h>
#endif

#ifdef HAVE_STD_IOSTREAM
using namespace std;
#endif

#include <stdlib.h> // exit, on_exit
#include <errno.h> // errno

#ifdef HAVE_UNISTD_H
#  include <unistd.h> // fork, umask, setsid, dup2, chdir, close
#endif

#ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h> // fork, umask, open
#endif

#ifdef HAVE_SYS_STAT_H
#  include <sys/stat.h> //open
#endif

#ifdef HAVE_FCNTL_H
#  include <fcntl.h> // open
#endif

#ifdef HAVE_SYSLOG_H
#  include <syslog.h> // openlog, syslog
#endif

#ifdef HAVE_STRING_H
#  include <string.h> // strerror
#endif

#ifdef HAVE_SIGNAL_H
#include <signal.h> // kill
#endif

#include <string>

// Forward declaration of omniORB::setLogFunction()
namespace omniORB {
  void setLogFunction(void (*logFunction)(const char*));
}

namespace Omniifr {

#define STRERR_FILE_LINE strerror(errno)<<" "<<__FILE__<<":"<<__LINE__

#define PIPE_READ  0
#define PIPE_WRITE 1
//////////////////////////////////////////////////////////////////////////////

/** Singleton - only at file scope. Although we initialize the value to NULL,
 * we don't trust that the runtime will actually initialise it.
 */
DaemonImpl daemon;

Daemon::Daemon(int&,char**&)
{
  // Initialise the DaemonImpl singleton.
  daemon._tracefile=NULL;
  daemon._foreground=false;
  daemon._pidfile=NULL;
  daemon._pipe[0]=daemon._pipe[1]=-1;
  daemon._havePidfile=false;
  daemon._haveParent=false;
  daemon._haveSyslog=false;
}
void Daemon::tracefile(const char* val) { daemon.tracefile(val); }
void Daemon::pidfile(const char* val)   { daemon.pidfile(val); }
void Daemon::foreground(bool val)       { daemon.foreground(val); }
void Daemon::daemonize()                { daemon.daemonize(); }
void Daemon::runningOk()                { daemon.runningOk(); }
Daemon::~Daemon()                       { daemon.shutdown(0); }

void shutdown0(void)       { daemon.shutdown(0); } ///< Param to atexit().
void shutdown2(int s,void*){ daemon.shutdown(s); } ///< Param to on_exit().

//////////////////////////////////////////////////////////////////////////////

DaemonImpl::DaemonImpl(){}


DaemonImpl::~DaemonImpl()
{
  delete[] _pidfile;
  delete[] _tracefile;
  _pidfile=NULL;
  _tracefile=NULL;
}


void DaemonImpl::tracefile(const char* val)
{
  _tracefile=::strdup(val);
}


void DaemonImpl::foreground(bool val)
{
  _foreground=val;
}


void DaemonImpl::pidfile(const char* val)
{
  string pidfileStr =val;
  if(pidfileStr[0]!='/')
      pidfileStr=string("/var/run/")+pidfileStr;
  DaemonImpl::_pidfile=::strdup(pidfileStr.c_str());
}


void DaemonImpl::initialize(int&,char**&)
{
  // Does nothing on Unix
}


void DaemonImpl::daemonize()
{
  // Register the shutdown function.
#ifdef HAVE_ON_EXIT
  if( ::on_exit(shutdown2,NULL) <0)
#else
  if( ::atexit(shutdown0) <0)
#endif
  {
    cerr<<"Failed to set exit handler."<<endl;
    ::exit(-1);
  }

  if(!_foreground)
  {
    this->fork();
  // ...now in the CHILD.
  }

  // Check & write the pidfile (if _pidfile is set).
  checkPidfileOrShutdown();
  writePidfile();

  // Change the file mode mask
  ::umask(0);
          
  // Change the current working directory
  if(::chdir("/")!=0)
  {
    cerr<<STRERR_FILE_LINE<<endl;
    ::exit(-1);
  }

  // If _tracefile is not set, then use syslog.
  if(_tracefile && _tracefile[0]!='\0')
  {
    redirectStreamsTo(_tracefile);
  }
  else
  {
#ifndef HAVE_OMNIORB3
#  ifdef LOG_PERROR
    ::openlog(PACKAGE_NAME ": ",LOG_PID|LOG_PERROR,LOG_DAEMON);
#  else
    ::openlog(PACKAGE_NAME ": ",LOG_PID,LOG_DAEMON);
#  endif
    _haveSyslog=true;
    omniORB::setLogFunction(DaemonImpl::log);
#else
    cerr<<"You must use option -t to set the file for trace messages."
      "\n(This is because omniORB3 cannot redirect messages to syslog.)"<<endl;
    ::exit(-1);
#endif
  }
} // end daemonize()


void DaemonImpl::runningOk()
{
  if(_haveParent)
  {
    _haveParent=false;
    notifyParent(0);
  }

  // No longer send syslog messages to stderr.
  if(_haveSyslog)
  {
#ifdef LOG_PERROR
    ::closelog();
    // FIXME: Possible race here? If a log message is sent right now.
    ::openlog(PACKAGE_NAME ": ",LOG_PID,LOG_DAEMON);
#endif
    redirectStreamsTo("/dev/null");
  }
}


void DaemonImpl::shutdown(int status)
{
  // Remove the pidfile.
  if(_havePidfile && _pidfile && 0!=::unlink(_pidfile))
  {
    cerr<<"Failed to remove pidfile '"<<_pidfile<<"': "
        <<STRERR_FILE_LINE<<endl;
    status=-1;
  }
  _havePidfile=false;

  // Close syslog.
  if(_haveSyslog)
  {
    _haveSyslog=false;
    ::closelog();
  }
  
  // Notify the parent.
  if(_haveParent)
  {
    _haveParent=false;
    notifyParent(status);
  }

  // outtahere...
}


void DaemonImpl::log(const char* message)
{
  // Cut off the redundant packageNamePrefix.
  static const char*  packageNamePrefix    ="omniIFR: ";
  static const size_t packageNamePrefixLen =::strlen(packageNamePrefix);
  if(0==::strncmp(message,packageNamePrefix,packageNamePrefixLen))
      message+=packageNamePrefixLen;
  // Send the message.
  ::syslog(LOG_INFO,message);
#ifndef LOG_PERROR
  // If we don't have LOG_PERROR, then we'll have to manually send
  // log messages to stderr.
  if(daemon._haveParent)
      cerr<<message<<flush;
#endif
}


void DaemonImpl::checkPidfileOrShutdown()
{
  if(!_pidfile)
      return;

  // Try to read pidfile.
  pid_t pidFromFile =0;
  struct stat buf;
  if(0==::stat(_pidfile,&buf))
  {
    if(!S_ISREG(buf.st_mode))
    {
      cerr<<"Pidfile '"<<_pidfile<<"' is not a regular file."<<endl;
      ::exit(-1);
    }
    try
    {
      ifstream infile(_pidfile);
      infile>>pidFromFile;
      infile.close();
    }
    catch(...)
    {
      cerr<<"Failed to read pidfile'"<<_pidfile<<"'."<<endl;
      ::exit(-1);
    }
  }
  else if(errno!=ENOENT)
  {
    cerr<<"Failed to stat pidfile '"<<_pidfile<<"': "
        <<STRERR_FILE_LINE<<endl;
    ::exit(-1);
  }

  // If process 'pidFromFile' is running then exit, else remove pidfile.
  if(pidFromFile>0)
  {
    if(0==::kill(pidFromFile,0)) // tests for running 'pidFromFile'.
    {
      cerr<<"Quitting because process "<<pidFromFile
          <<" defined in pidfile '"<<_pidfile<<"'"
          <<" is already running."<<endl;
      ::exit(-1);
    }
    else if(errno!=ESRCH)
    {
      cerr<<"Failed to test for process "<<pidFromFile
          <<" defined in pidfile '"<<_pidfile<<"': "
          <<STRERR_FILE_LINE<<endl;
      ::exit(-1);
    }
  }
}


void DaemonImpl::writePidfile()
{
  if(_pidfile)
  {
    try
    {
#ifdef FSTREAM_OPEN_PROT
      ofstream outfile(_pidfile,ios::out|ios::trunc,0644);
#else
      ofstream outfile(_pidfile,ios::out|ios::trunc);
#endif
      outfile<<::getpid()<<endl;
      outfile.close();
      // Tell shutdown() that the pidfile needs to be cleared away.
      _havePidfile=true;
    }
    catch(...)
    {
      cerr<<"Failed to write pidfile '"<<_pidfile<<"'."<<endl;
      ::exit(-1);
    }
  }
}


void DaemonImpl::fork()
{
  if( ::pipe(_pipe) <0)
  {
    cerr<<"Failed to open pipe: "<<STRERR_FILE_LINE<<endl;
    ::exit(-1);
  }

  // Fork off from the parent process
  pid_t pid =::fork();
  if(pid<0)
  {
    cerr<<STRERR_FILE_LINE<<endl;
    ::exit(-1);
  }
  else if(pid>0)
  {
    //
    // Now in the PARENT
    //

    // Close the write end of the pipe.
    if( ::close(_pipe[PIPE_WRITE]) <0)
        cerr<<"Failed to close pipe: "<<STRERR_FILE_LINE<<endl;

    ::_exit(waitForChild()); // Exit without flushing buffers
  }

  //
  // ...now in the CHILD.
  //

  _haveParent=true;

  // Close the read end of the pipe
  if( ::close(_pipe[PIPE_READ]) <0)
      cerr<<"Failed to close pipe: "<<STRERR_FILE_LINE<<endl;

  // Create a new SID for the child process
  pid_t sid =::setsid();
  if(sid<0)
  {
    cerr<<STRERR_FILE_LINE<<endl;
    ::exit(-1);
  }
}


void DaemonImpl::redirectStreamsTo(const char* filename)
{
  if(openFileFor(STDIN_FILENO,"/dev/null",O_RDONLY)<0)
  {
    cerr<<"Failed to open /dev/null for STDIN: "<<STRERR_FILE_LINE<<endl;
    ::exit(-1);
  }
  if(openFileFor(STDOUT_FILENO,filename,O_WRONLY|O_CREAT|O_APPEND)<0)
  {
    cerr<<"Failed to open "<<filename<<" for STDOUT: "<<STRERR_FILE_LINE<<endl;
    ::exit(-1);
  }
  if(openFileFor(STDERR_FILENO,filename,O_WRONLY|O_CREAT|O_APPEND)<0)
  {
    cerr<<"Failed to open "<<filename<<" for STDERR: "<<STRERR_FILE_LINE<<endl;
    ::exit(-1);
  }
}


int DaemonImpl::openFileFor(int fd, const char* filename, int flags)
{
  int newfd =::open(filename,flags,0644);
  if(newfd<0)
      return -1;
  if(newfd==fd)
      return fd;
  if(::dup2(newfd,fd)<0) // replace fd with a copy of newfd
      return -1;
  ::close(newfd);
  return fd;
}


int DaemonImpl::waitForChild()
{
  int status =-1;
  ssize_t bytes =::read(_pipe[PIPE_READ],&status,sizeof(status));
  if(bytes<sizeof(status))
  {
    status=-1;
    if(bytes<0)
       cerr<<"Parent failed to read result from pipe: "<<STRERR_FILE_LINE<<endl;
  }
  if( ::close(_pipe[PIPE_READ]) !=0)
      cerr<<"Failed to close pipe: "<<STRERR_FILE_LINE<<endl;

  return status;
}


void DaemonImpl::notifyParent(int status)
{
  ssize_t r =::write(_pipe[PIPE_WRITE],&status,sizeof(status));
  if(r<sizeof(status))
  {
    if(r<0)
        cerr<<"read() failed while writing return value to pipe: "
            <<STRERR_FILE_LINE<<endl;
    else
        cerr<<"write() too short while writing return value from pipe: "
            <<STRERR_FILE_LINE<<endl;
  }
  if( ::close(_pipe[PIPE_WRITE]) !=0)
      cerr<<"Failed to close pipe: "<<STRERR_FILE_LINE<<endl;
}

} // end namespace Omniifr
