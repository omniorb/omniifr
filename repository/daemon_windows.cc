//                            Package   : omniIFR
//  daemon_windows.cc         Created   : 2004/07/23
//                            Author    : Alex Tingle
//
//    Copyright (C) 2004 Alex Tingle
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
#include "daemon_windows.h"
#include "main.h"

#define NEED_PACKAGE_INFO
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

using namespace std;

#include <fstream>
#include <stdlib.h> // exit, on_exit
#include <errno.h> // errno
#include <string>
#include <vector>

#define AS_STR_2(x) #x
#define AS_STR_1(x) AS_STR_2(x)
/** Generates a string literal that describes the filename and line number. */
#define HERE __FILE__ ":" AS_STR_1(__LINE__)

// Forward declaration of omniORB::setLogFunction()
namespace omniORB {
  void setLogFunction(void (*logFunction)(const char*));
}

namespace Omniifr {

/** Utility class, contains functions that Windows should have, but doesn't. */
class Win
{
public:
  static const char* strerror(DWORD e)
  {
    LPVOID buf;
    ::FormatMessage(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
      NULL,
      e,
      MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
      (LPTSTR)&buf,
      0, NULL
    );
    return (const char*)(buf);
  }

  static void perror(const char* s=NULL)
  {
    if(s)
    {
      Service::log(s);
      Service::log(": ");
    }
    Service::log(Win::strerror(::GetLastError()));
  }
};

/** Opens a windows registry key, and closed it upon destruction.
 * Upon failure, it emits an error message and then quits.
 * Why do I have to write this class myself??
 */
class RegistryKey
{
  HKEY _hkey;
  bool _open;
private:
  RegistryKey(); ///< No implementation
  RegistryKey(HKEY hkey, bool open=true):_hkey(hkey),_open(open){}
public:
  RegistryKey(RegistryKey& right);
  RegistryKey(HKEY hkey, const char* subkey, REGSAM samDesired=KEY_QUERY_VALUE);
  ~RegistryKey();
  operator bool() const { return _open; }
  int setValueStr(const char* name, const char* data);
  char* queryValueStr(const char* name, const int maxlen=2048) const;
};

/** Copy constructor, adopts ownership. */
RegistryKey::RegistryKey(RegistryKey& right):
  _hkey(right._hkey),_open(right._open)
{
  right._open=false;
}

/** Constructor, opens the key. */
RegistryKey::RegistryKey(
  HKEY hkey,
  const char* subkey,
  REGSAM samDesired
):_hkey(), _open(false)
{
  long ret=::RegOpenKeyEx(hkey,subkey,0,samDesired,&_hkey);
  ::SetLastError(ret);
  if(ret==ERROR_SUCCESS)
      _open=true;
}

/** Destructor, closes the key. */
RegistryKey::~RegistryKey()
{
  // Windows - why use two lines, when seven will do??
  // RegCloseKey() does not set last error, so complexity ensues...
  if(_open)
  {
    long ret =::RegCloseKey(_hkey);
    ::SetLastError(ret);
    if(ret!=ERROR_SUCCESS)
        Win::perror("Warning at " HERE);
  }
}

int RegistryKey::setValueStr(const char* name, const char* data)
{
  long ret=::RegSetValueEx(
      _hkey,name,0,REG_SZ,
      (const BYTE*)(data),
      1+::strlen(data)
    );
  ::SetLastError(ret);
  if(ret==ERROR_SUCCESS)
      return 0;
  else
      return 1;
}

char* RegistryKey::queryValueStr(const char* name, const int maxlen) const
{
  char* result =NULL;
  char* buf =new char[maxlen];
  DWORD len =maxlen;

  long ret=::RegQueryValueEx(_hkey,name,NULL,NULL,(LPBYTE)buf,&len);
  ::SetLastError(ret);
  if(ret==ERROR_SUCCESS && len<=maxlen)
      result=::strdup(buf); // MSVC6 has no strndup()!!
  delete[] buf;
  return result;
}
//////////////////////////////////////////////////////////////////////////////

/** Singleton - only at file scope. */
static Service service;

Daemon::Daemon(int& argc,char**& argv)  { service.start(argc,argv); }
void Daemon::tracefile(const char* val) { service.tracefile(val); }
void Daemon::pidfile(const char* val)   { service.pidfile(val); }
void Daemon::foreground(bool val)       { service.foreground(val); }
void Daemon::daemonize()                { service.daemonize(); }
void Daemon::runningOk()                { service.runningOk(); }
Daemon::~Daemon()                       { service.shutdown(); }

void shutdown0(void){ service.shutdown(); } ///< Param to atexit()

///////////////////////////////////////////////////////////////////////////////

Service::Service():
  _tracefile(NULL),
  _regSubKey("SYSTEM\\CurrentControlSet\\Services\\" PACKAGE_NAME),
  _serviceRunning(false),
  _callCount(0),
  _parameters(NULL),
  _argv(NULL),
  _logstream(&cerr),
  _serviceStatusHandle()
{}


Service::~Service()
{
  delete[] _tracefile;
  delete[] _parameters;
  delete[] _argv;
  if(_logstream!=&cerr)
      delete _logstream;
}


void Service::tracefile(const char* val)
{
  delete[] _tracefile;
  _tracefile=::strdup(val);
}


void Service::pidfile(const char* val)
{
  Service::log("Option -P not supported on windows.\n");
  ::exit(1);
}


void Service::foreground(bool val)
{
  Service::log("Option -f not supported on windows.\n");
  ::exit(1);
}


void Service::start(int& argc,char**& argv)
{
  ++_callCount;
  if(_callCount>1)
  {
    // This is a re-entrant call. We are inside 'ServiceMain()'.
    setArgcArgv(argc,argv); // Set argv & argc from the registry.
    _serviceStatusHandle=
      ::RegisterServiceCtrlHandler(
        PACKAGE_NAME,
        (LPHANDLER_FUNCTION)Service::ctrlHandler
      );
    if(!_serviceStatusHandle)
        ::exit(1);
    if(! setServiceStatus(SERVICE_START_PENDING,NO_ERROR,0,1,3000) )
        ::exit(1);
    _serviceRunning=true;
    // ...and return to main().
  }
  else if(argc>=2 && 0==::strcmp(argv[1],"service"))
  {
    // Start service.
    char* name =::strdup(PACKAGE_NAME);
    SERVICE_TABLE_ENTRY servicetable[]=
    {
      {name,(LPSERVICE_MAIN_FUNCTION)::main},
      {NULL,NULL}
    };
    if(! ::StartServiceCtrlDispatcher(servicetable) )
    {
      Win::perror(HERE);
      ::exit(1);
    }
    ::exit(0);
  }
  else if(argc>=2 && 0==::strcmp(argv[1],"install"))
  {
    install(argc,argv);
    cout<<"Service '" PACKAGE_NAME "' installed OK."<<endl;
    ::exit(0);
  }
  else if(argc>=2 && 0==::strcmp(argv[1],"uninstall"))
  {
    uninstall();
    cout<<"Service '" PACKAGE_NAME "' removed."<<endl;
    ::exit(0);
  }
  else if(argc>=2 && 0==::strcmp(argv[1],"getoptions"))
  {
    readParameters();
    cout<<_parameters<<endl;
    ::exit(0);
  }
  else if(argc>=2 && 0==::strcmp(argv[1],"setoptions"))
  {
    writeParameters(argc,argv);
    ::exit(0);
  }
  else if(argc>=2 && 0==::strcmp(argv[1],"run"))
  {
    setArgcArgv(argc,argv); // Set argv & argc from the registry.
  }
  else
  {
    ; // Just run the program in the foreground.
  }
}


void Service::daemonize()
{
  if(_tracefile && _tracefile[0]!='\0')
  {
    _logstream=new ofstream(_tracefile,ios::out|ios::app);
    omniORB::setLogFunction(Service::log);
  }

  // Register the shutdown function.
  if( ::atexit(shutdown0) <0) // Windows has atexit()
  {
    Service::log("Failed to set exit handler.");
    ::exit(-1);
  }
}


void Service::runningOk()
{
  if(_serviceRunning)
  {
    if(! setServiceStatus(SERVICE_RUNNING,NO_ERROR,0,0,0) )
        ::exit(1);
  }
}


void Service::shutdown()
{
  if(_logstream!=&cerr)
  {
    delete _logstream;
    _logstream=&cerr;
  }
  if(_serviceRunning)
  {
    setServiceStatus(SERVICE_STOPPED,NO_ERROR,0,0,0);
    _serviceRunning=false;
  }
}

// static callback
void Service::log(const char* message)
{
  (*service._logstream)<<message<<flush;
}

// static callback
void Service::ctrlHandler(DWORD controlCode)
{
  switch(controlCode)
  {  
  case SERVICE_CONTROL_SHUTDOWN:
  case SERVICE_CONTROL_STOP:
      ::Omniifr_Orb_shutdown(controlCode);
      service.setServiceStatus(SERVICE_STOP_PENDING,NO_ERROR,0,1,6000);
      break;
  case 128: // User defined code.
      ::Omniifr_Orb_bumpTraceLevel(controlCode);
      break;
  default:
      break;
  }
}


void Service::setArgcArgv(int& argc,char**& argv)
{
  readParameters();
  vector<char*> args;
  char* param =::strtok(_parameters,"\t ");
  while(param)
  {
    args.push_back(param);
    param=::strtok(NULL,"\t ");
  }
  if(!args.empty())
  {
    _argv=new char*[argc+args.size()]; // deleted by ~Service()
    int i=0;
    _argv[i++]=argv[0];
    for(int j=0; j<args.size(); ++j)
        _argv[i++]=args[j];
    for(int k=1; k<argc; ++k)
        _argv[i++]=argv[k];
    argv=_argv;
    argc=i;
  }
}


void Service::install(int argc,char** argv) const
{
  //
  // Install service
  char exe_file_name[MAX_PATH];
  if(0== ::GetModuleFileName(0, exe_file_name, MAX_PATH) )
  {
    Win::perror(HERE);
    ::exit(1);
  }

  string command =string(exe_file_name)+" service";

  SC_HANDLE scmanager =::OpenSCManager(0,0,SC_MANAGER_CREATE_SERVICE);
  if(!scmanager)
  {
    Win::perror(HERE);
    ::exit(1);
  }
  SC_HANDLE service =
    ::CreateService(
      scmanager,
      PACKAGE_NAME,
      "CORBA Event Daemon",
      SERVICE_ALL_ACCESS,
      SERVICE_WIN32_OWN_PROCESS,
      SERVICE_AUTO_START,
      SERVICE_ERROR_NORMAL,
      command.c_str(),
      0,0,0,0,0
    );
  if(!service)
  {
    Win::perror(HERE);
    ::exit(1);
  }
  if(0== ::CloseServiceHandle(service) )
  {
    Win::perror(HERE);
    ::exit(1);
  }
  if(0== ::CloseServiceHandle(scmanager) )
  {
    Win::perror(HERE);
    ::exit(1);
  }

  //
  // Set the service's parameters & description.
  writeParameters(argc,argv);
  RegistryKey rkey(HKEY_LOCAL_MACHINE,_regSubKey,KEY_SET_VALUE);
  if(!rkey)
  {
    Win::perror("Can't open registry key at " HERE);
    ::exit(1);
  }
  if(0!= rkey.setValueStr("Description",
           "Asynchronous broadcast channels for CORBA applications.") )
  {
    Win::perror("Can't set registry value 'Description' at " HERE);
    ::exit(1);
  }
}


void Service::uninstall() const
{
  SC_HANDLE scmanager =::OpenSCManager(0,0,SC_MANAGER_CREATE_SERVICE);
  if(!scmanager)
  {
    Win::perror(HERE);
    ::exit(1);
  }
  SC_HANDLE service =
    ::OpenService(
      scmanager,
      PACKAGE_NAME,
      SC_MANAGER_ALL_ACCESS
    );
  if(!service)
  {
    Win::perror(HERE);
    ::exit(1);
  }
  if(0== ::DeleteService(service) )
  {
    Win::perror(HERE);
    ::exit(1);
  }
  if(0== ::CloseServiceHandle(service) )
  {
    Win::perror(HERE);
    ::exit(1);
  }
  if(0== ::CloseServiceHandle(scmanager) )
  {
    Win::perror(HERE);
    ::exit(1);
  }
}


void Service::readParameters()
{
  RegistryKey rkey(HKEY_LOCAL_MACHINE,_regSubKey);
  if(!rkey)
  {
    Win::perror("Can't open registry key at " HERE);
    ::exit(1);
  }
  _parameters=rkey.queryValueStr("Parameters"); // deleted by ~Service()
  if(_parameters==NULL)
  {
    Win::perror("Can't get Parameters at " HERE);
    ::exit(1);
  }
}


void Service::writeParameters(int argc, char** argv) const
{
  RegistryKey rkey(HKEY_LOCAL_MACHINE,_regSubKey,KEY_SET_VALUE);
  if(!rkey)
  {
    Win::perror("Can't open registry key at " HERE);
    ::exit(1);
  }
  string parameters ="";
  for(int i=2; i<argc; ++i)
  {
    if(!parameters.empty())
        parameters+=" ";
    parameters+=argv[i];
  }
  if(0!= rkey.setValueStr("Parameters",parameters.c_str()) )
  {
    Win::perror("Can't set registry value 'Parameters' at " HERE);
    ::exit(1);
  }
}


bool Service::setServiceStatus(
  DWORD currentState,
  DWORD win32ExitCode,
  DWORD serviceSpecificExitCode,
  DWORD checkPoint,
  DWORD waitHint)
{
  SERVICE_STATUS s;
  s.dwServiceType            =SERVICE_WIN32_OWN_PROCESS;
  s.dwCurrentState           =currentState;
  s.dwServiceSpecificExitCode=serviceSpecificExitCode;
  s.dwCheckPoint             =checkPoint;
  s.dwWaitHint               =waitHint;

  if(currentState==SERVICE_START_PENDING)
      s.dwControlsAccepted=0;
  else
      s.dwControlsAccepted=SERVICE_ACCEPT_STOP|SERVICE_ACCEPT_SHUTDOWN;

  if(serviceSpecificExitCode==0)
      s.dwWin32ExitCode=win32ExitCode;
  else
      s.dwWin32ExitCode=ERROR_SERVICE_SPECIFIC_ERROR;

  return (0!= ::SetServiceStatus(_serviceStatusHandle,&s) );
}

} // end namespace Omniifr
