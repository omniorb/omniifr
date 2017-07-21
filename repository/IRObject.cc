//                            Package   : omniIFR
//  IRObject.cc               Created   : 2004/02/22
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
#include "IRObject.h"

#include "Repository.h"
#include "PersistNode.h"

#include <algorithm>

#ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h> // getpid
#endif

#ifdef HAVE_UNISTD_H
#  include <unistd.h>    // getpid
#elif defined(HAVE_PROCESS_H)
# include <process.h>
#endif

#include <stdio.h>     // sprintf

namespace Omniifr {

/** Helper method. Just gets the object's POA, POA name, OID &
 *  Stringified OID. */
void getPoaAndOid(
  IRObject_impl*                 servant,
  PortableServer::POA_var&       poa,
  CORBA::String_var&             poaName,
  PortableServer::ObjectId_var&  oid,
  CORBA::String_var&             oidStr
)
{
  using namespace PortableServer;
  poa=servant->_default_POA();
  poaName=poa->the_name();
  try
  {
    oid=poa->servant_to_id(servant);
  }
  catch(POA::ServantNotActive& ex)
  {
    cerr<<"Can't get ObjectID.\nPOA '"<<poaName.in()
        <<"' says it is not active."<<endl;
  }
  catch(POA::WrongPolicy& ex)
  {
    cerr<<"Can't get ObjectID.\nPOA '"<<poaName.in()
        <<"' has wrong policy for servant_to_id()."<<endl;
    ::exit(1); // Programming error - so quit.
  }

  try
  {
    oidStr=ObjectId_to_string(oid.in());
  }
  catch(CORBA::BAD_PARAM& ex)
  {
    cerr<<"Can't convert ObjectID to string.\n"
      "BAD_PARAM" IFELSE_OMNIORB4(": "<<ex.NP_minorString(),) <<endl;
  }
}

//
//

void IRObject_impl::destroy()
{
  bool dependencyPreventsDestruction =true;

  set<const IRObject_impl*> localDeps;
  this->dependentObjectSet(localDeps);

  if(localDeps.empty())
  {
    dependencyPreventsDestruction=false;
  }
  else
  {
    set<const IRObject_impl*> localObjs;
    this->containedObjectSet(localObjs);

    set<const IRObject_impl*> externalDeps;
    set_difference(
      localDeps.begin(),localDeps.end(),
      localObjs.begin(),localObjs.end(),
      inserter(externalDeps,externalDeps.end())
    );
    if(externalDeps.empty())
        dependencyPreventsDestruction=false;
  }

  if(dependencyPreventsDestruction)
  {
    // Specification dictates the following exception:
    throw CORBA::BAD_INV_ORDER(
      IFELSE_OMNIORB4(omni::BAD_INV_ORDER_DependencyPreventsDestruction,1),
      CORBA::COMPLETED_NO
    );
  }
  else
  {
    this->uncheckedDestroy();
    this->deactivateObject();
    _remove_ref(); // Kill the constructor's reference to this.
  }
}

PortableServer::POA_ptr IRObject_impl::_default_POA()
{
  return PortableServer::POA::_duplicate(Repository_impl::inst()._poa.in());
}

void IRObject_impl::activateObject()
{
  // Generate a new unique ID.
  static long  count=0;
  static omni_mutex  mutex;
  int  mypid =getpid(); // MS VC++6 doesn't have type pid_t!
  unsigned long  sec,nsec;
  omni_thread::get_time(&sec,&nsec); // More portable than just time().
  char buf[128];
  {
    omni_mutex_lock l(mutex);
    sprintf(buf,"%lx.%d.%lx",++count,mypid,sec);
  }
  activateObjectWithId(buf);
}

void IRObject_impl::activateObjectWithId(const char* oidStr)
{
  using namespace PortableServer;
  PortableServer::POA_var poa     =_default_POA();
  CORBA::String_var       poaName =poa->the_name();
  DB(9,"Activating object "<<poaName.in()<<"/"<<oidStr);
  try
  {
    ObjectId_var oid =string_to_ObjectId(oidStr);
    poa->activate_object_with_id(oid.in(),this);
    _activated=true;
  }
  catch(CORBA::BAD_PARAM& ex)
  {
    cerr<<"Can't activate "<<oidStr<<".\n"
      "BAD_PARAM" IFELSE_OMNIORB4(": "<<ex.NP_minorString(),) <<endl;
  }
  catch(POA::ServantAlreadyActive& ex)
  {
    cerr<<"Can't activate "<<oidStr<<".\nServant is already active."<<endl;
  }
  catch(POA::ObjectAlreadyActive& ex)
  {
    cerr<<"Can't activate "<<oidStr<<".\nObject is already active."<<endl;
  }
  catch(POA::WrongPolicy& ex)
  {
    cerr<<"Can't activate "<<oidStr<<".\nPOA '"<<poaName.in()
        <<"' has wrong policy for activate_object_with_id()."<<endl;
    exit(1); // Programming error - so quit.
  }
}


void IRObject_impl::deactivateObject()
{
  using namespace PortableServer;
  POA_var            poa;
  CORBA::String_var  poaName;
  ObjectId_var       oid;
  CORBA::String_var  oidStr;
  getPoaAndOid(this,poa,poaName,oid,oidStr);

  try
  {
    DB(10,"Deactivating object "<<poaName<<"/"<<oidStr.in());
    poa->deactivate_object(oid.in());
  }
  catch(POA::ObjectNotActive& ex)
  {
    cerr<<"Can't deactivate "<<oidStr<<".\nObject is not active."<<endl;
  }
  catch(POA::WrongPolicy& ex)
  {
    cerr<<"Can't deactivate "<<oidStr<<".\nPOA '"<<poaName.in()
        <<"' has wrong policy for deactivate_object()."<<endl;
    exit(1); // Programming error - so quit.
  }
}


void IRObject_impl::reincarnate(const PersistNode& node)
{
  cerr<<"Failed attempt to reincarnate. Wrong class."<<endl;
  node.output(cerr,"Error Node");
}


void IRObject_impl::outputOid(ostream &os)
{
  using namespace PortableServer;
  POA_var            poa;
  CORBA::String_var  poaName;
  ObjectId_var       oid;
  CORBA::String_var  oidStr;
  getPoaAndOid(this,poa,poaName,oid,oidStr);

  os<<oidStr.in();
}


bool IRObject_impl::checkReadonly() const
{
  if(_activated && Repository_impl::inst().readonly())
  {
    DB(20,"rejected attempt to change readonly repository.")
    throw CORBA::NO_PERMISSION();
  }
}


} // end namespace Omniifr
