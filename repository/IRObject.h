//                            Package   : omniIFR
//  IRObject.h                Created   : 2004/02/22
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
#ifndef OMNIIFR__IROBJECT_H
#define OMNIIFR__IROBJECT_H

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <omniORB4/CORBA.h>

#include <assert.h>
#include <set>

#ifdef HAVE_IOSTREAM
#  include <iostream>
#else
#  include <iostream.h>
#endif

#ifdef HAVE_STD_STL
using namespace std;
#endif

#ifdef HAVE_OMNIORB4
#  define IFELSE_OMNIORB4(omniORB4_code,default_code) omniORB4_code
#else
#  define IFELSE_OMNIORB4(omniORB4_code,default_code) default_code
#endif

#define DB(l,x) {if(omniORB::trace(l)){omniORB::logger log("omniIFR: ");log<<x<<"\n";}}

/** Helper, used by attribute getters. */
#define getAttr(x) if(x) return x->_this(); else throw CORBA::BAD_INV_ORDER();

namespace Omniifr {

class Repository_impl;
class IRObject_impl;
class PersistNode;

/** Base class for  Dependency objects. */
class DependencyBase
{
public:
  DependencyBase(IRObject_impl* owner):_owner(owner){assert(owner);}
  virtual ~DependencyBase(){}
  inline IRObject_impl* owner() const {return _owner;}
private:
  DependencyBase(); ///< No default constructor.
  IRObject_impl* _owner;
};


class IRObject_impl :
  public virtual PortableServer::RefCountServantBase,
  public virtual POA_CORBA::IRObject
{
public: // CORBA interface methods
  virtual CORBA::DefinitionKind def_kind() =0;
  virtual void destroy();

private:
  multiset<const DependencyBase*> _dependencies;
  bool _activated; ///< Set to TRUE when activateObject...() is called.
  
public:
  IRObject_impl():_activated(false){}
  virtual ~IRObject_impl()
  {
    if(!_dependencies.empty())
        DB(1,"Destructed object still has dependencies. Wait for it...")
  }

  /** A new unique ID is assigned. */
  void activateObject();
  void activateObjectWithId(const char* oidStr);
  void deactivateObject(); 

  /** Returns a reference to this servant's default POA. The default
   * implementation returns a reference to the Repository's _poa.
   */
  virtual PortableServer::POA_ptr _default_POA();

  /** An an incoming dependency to this object. The object dependency->owner()
   * needs this object. This object cannot be destroyed while the dependency
   * exists.
   */
  void depend(const DependencyBase* dependency)
  {
    _add_ref();
    _dependencies.insert(dependency);
  }
  /** Remove a dependency to this object. This object can only be destroyed
   * when all dependencies have been removed.
   */
  void undepend(const DependencyBase* dependency)
  {
    _dependencies.erase(dependency);
    _remove_ref();
  }

  /** 'result' is set to the set union of 'result's initial value and the
   * set of all objects that depend upon this object (or its children).
   */
  virtual void dependentObjectSet(set<const IRObject_impl*>& result) const
  {
    for(multiset<const DependencyBase*>::const_iterator i=_dependencies.begin();
        i!=_dependencies.end();
        ++i)
    {
      result.insert((**i).owner());
    }
  }

  /** 'result' is set to the set union of 'result's initial value and the set
   * of all objects contained by this object, plus this object itself.
   */
  virtual void containedObjectSet(set<const IRObject_impl*>& result) const
  {
    result.insert(this);
  }

  /** Destroys this object, without first checking for dependencies. */
  virtual void uncheckedDestroy()=0;

  /** Re-create the repository from information saved in the log file. */
  virtual void reincarnate(const PersistNode& node);

  /** Save this object's state to a stream. */
  virtual void output(ostream &os) =0;
  
  /** Save this object's OID to a stream. */
  void outputOid(ostream &os);

  /** Throws CORBA::NO_PERMISSION if the Repository is readonly, but only if
   * _activated is also TRUE.
   *
   * This method is called by servants before they change the repository. It
   * forbids changes to readonly repositories, except to ojects that are not yet
   * activated - they may need to call CORBA methods such as
   * Contained::name(X) as part of their (pre-activation) set-up.
   */
  bool checkReadonly() const;
};

} // end namespace Omniifr

#endif // OMNIIFR__IROBJECT_H
