//                            Package   : omniIFR
//  main.cc                   Created   : 2004/02/22
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

#include "main.h"

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#if defined(HAVE_SIGNAL_H) && defined(HAVE_SIGSET)
#  include <signal.h>
#  define SIGSET(sig,func) ::sigset(sig,func)
#elif defined(HAVE_SIGNAL_H)
#  include <signal.h>
#  define SIGSET(sig,func) ::signal(sig,func)
#endif

#include "Repository.h"
#include "Persist.h"
#include "daemon.h"

CORBA::ORB_var orb; ///< The global ORB
bool readonly=false;  ///< Is the IFR to be readonly - set by options().
Omniifr::Persist* persist =NULL;
int  checkpointPeriodSec=900; ///< How often to checkpoint - set by options().
omni_semaphore shutdownSemaphore(0);

#ifdef HAVE_GETOPT
#  include <unistd.h>
extern char* optarg;
extern int optind;

void usage(int argc, char** argv);
void version();

void options(
  int               argc,
  char**            argv,
  Omniifr::Daemon&  daemon,
  Omniifr::Persist& persist
)
{
  int c;
  while ((c = getopt(argc,argv,"fl:P:rt:Vh")) != EOF)
  {
    switch (c)
    {
      case 'f': daemon.foreground(true);
                break;

      case 'P': daemon.pidfile(optarg);
                break;

      case 'l': persist.setdir(optarg);
                break;

      case 'r': readonly=true;
                break;

   // Informational options
      case 't': daemon.tracefile(optarg);
                break;

      case 'V': version();
                break;

      case 'h':
      default : usage(argc,argv);
                break;
    }
  }
}
#endif

int main(int argc, char* argv[])
{
  Omniifr::Daemon daemon(argc,argv);

  const char* action=""; // Use this variable to help report errors.
  try
  {
    action="initialising ORB";
    orb=CORBA::ORB_init(argc,argv);
    
    action="constructing Persist";
    persist =new Omniifr::Persist(readonly? 0: checkpointPeriodSec);

#ifdef HAVE_GETOPT
    action="processing command line options";
    options(argc,argv,daemon,*persist);
#endif

    action="daemonizing";
    daemon.daemonize();


    action="initialising Repository";
    Omniifr::Repository_impl::inst().init( orb.in(), readonly, persist );
    if(readonly)
    {
      action="destroying Persist";
      delete persist;
      persist=NULL;
    }

#ifdef HAVE_SIGNAL_H
    SIGSET(SIGINT , ::Omniifr_Orb_shutdown);
    SIGSET(SIGTERM, ::Omniifr_Orb_shutdown);
#  ifdef SIGUSR1
    SIGSET(SIGUSR1, ::Omniifr_Orb_bumpTraceLevel);
#  endif
#  ifdef SIGUSR2
    SIGSET(SIGUSR2, ::Omniifr_Ifr_checkpoint);
#  endif
#  ifdef SIGPIPE
    SIGSET(SIGPIPE, SIG_IGN); // Ignore broken pipes
#  endif
#endif

    action="closing parent process";
    daemon.runningOk();

    // Park the main thread.
    // Do this by waiting on a semaphore rather than calling ORB_run() - 
    // this makes it safe for a signal to wake us up.
    action="running ORB";
    cout<<action<<'.'<<endl;
    shutdownSemaphore.wait();
    
    // SIGTERM or whatever has triggered shutdown.
    DB(1,"Shutdown requested.")
    action="shutting down ORB";
    orb->shutdown(1); // Synchronous shutdown
    action="destroying ORB";
    orb->destroy(); // clean up
    if(persist)
    {
      action="destroying Persist";
      delete persist;
    }
    return 0;

  }
  catch(CORBA::SystemException& ex)
  {
    cerr<<"Failed while "<<action<<"."
      IFELSE_OMNIORB4(" "<<ex._name()<<" ("<<ex.NP_minorString()<<")",0) <<endl;
  }
  catch(CORBA::Exception& ex)
  {
    cerr<<"Failed while "<<action<<"." IFELSE_OMNIORB4(" "<<ex._name(),0) <<endl;
  }
  return 1;
}

//
// Signal handlers.
//  
    
extern "C"
{   
  void Omniifr_Orb_shutdown(int signum)
  {
    // Wake up the main thread.
    shutdownSemaphore.post();
  }

  void Omniifr_Orb_bumpTraceLevel(int signum)
  {
    omniORB::traceLevel=(omniORB::traceLevel+5)%45;
    DB(0,"TRACE LEVEL BUMPED TO "<<omniORB::traceLevel<<" BY SIGNAL "<<signum)
  }

  void Omniifr_Ifr_checkpoint(int signum)
  {
    if(readonly)
    {
      DB(1,"Checkpoint request ignored: Interface Repository is read only.")
    }
    else
    {
      DB(1,"Checkpoint requested.")
      assert(persist!=NULL);
      persist->checkpoint();
    }
  }
}

