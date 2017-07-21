//                            Package   : omniIFR
//  Contained.cc              Created   : 2004/02/22
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
#include "Contained.h"

#include "Container.h"
#include "Repository.h"
#include "IdentifierUtil.h"

#include <string>
#include <string.h>

namespace Omniifr {

void Contained_impl::init(
  const char*     id,
  const char*     name,
  const char*     version,
  Container_impl* defined_in,
  int             index
)
{
  _index=index;
  _version=version;
  _defined_in=defined_in;

  assert(id);
  assert(name);
  assert(version);
  assert(defined_in);
  // containing_repository //?? why isn't this a member of this class?

  DB(5,"Contained_impl::init("<<id<<","<<name<<","<<version<<")")

  // Note: The order of the following three actions is important. Each may
  // throw BAD_PARAM, in which case the changes made so far must be backed out.

  int where =0; // Only for error reporting (see catch)
  try
  {

    // name() requires _defined_in to be set. Has no side-effects.
    this->name(name);
    ++where;

    // External-effect: adds id into the Repository.
    this->id(id);
    ++where;

    // External-effects:
    //  - adds 'this' to container's list;
    //  - calls _defined_in->_add_ref()
    // BUT only if the call completes succcessfully.
    _defined_in->addContained(this);

  }
  catch(CORBA::BAD_PARAM&) // Name already in use (or bad name)
  {
    switch(where)
    {
    case 0:
        DB(5,"Contained_impl::init() - failed to define name as "<<name)
        break;
    case 1:
        DB(5,"Contained_impl::init() - failed to define id as "<<id)
        break;
    case 2:
        DB(5,"Contained_impl::init() - failed to add to container.")
        // Remove the RID from the Repository.
        Repository_impl::inst().removeId(_id.in());
        break;
    default: assert(0);
    }

    // "_defined_in->addContained(this)" never succeeded, so we are
    // not defined anywhere --> uncheckedDestroy won't try to undefine us.
    _defined_in=NULL;
    throw;
  }
}

char* Contained_impl::id() 
{
  return CORBA::string_dup(_id);
}

void Contained_impl::id(const char* v)
{
  checkReadonly();
  assert(v);

  if(!v[0]) // empty string? invalid RID!
      throw CORBA::BAD_PARAM(
        IFELSE_OMNIORB4(omni::BAD_PARAM_InvalidRepositoryId,16),
        CORBA::COMPLETED_NO
      );
  if(_id.in() && 0==strcmp(_id,v)) // Unchanged? do nothing!
      return;
  if(Repository_impl::inst().findId(v)) // RID already in use.
      throw CORBA::BAD_PARAM(
        IFELSE_OMNIORB4(omni::BAD_PARAM_RIDAlreadyDefinedInIfR,2),
        CORBA::COMPLETED_NO
      );

  // Do it.
  if(_id.in())
      Repository_impl::inst().removeId(_id.in());
  _id=v; // Change the id
  Repository_impl::inst().addId(v,this);
}

char* Contained_impl::name() 
{
  return CORBA::string_dup(_name);
}

void Contained_impl::name(const char* v) 
{
  checkReadonly();
  assert(v);
  assert(_defined_in);

  IdentifierUtil::checkInvalid(v);
  switch(IdentifierUtil::compare(_name.in(),v))
  {
    case IdentifierUtil::equalMatch:
        break; // Unchanged -> do nothing!

    case IdentifierUtil::equivalentMatch:
      {
        _name=v; // Equivalent name -> just change it!
        updateAbsoluteName();
      }
      break;

    case IdentifierUtil::noMatch:
      {
        // Check that the new name is not already in use (case INsensitive)
        Contained_impl* match =_defined_in->lookupServant(v,false);
        if(match) // Name already in use.
            throw CORBA::BAD_PARAM(
              IFELSE_OMNIORB4(omni::BAD_PARAM_ValueFactoryFailure,1),
              CORBA::COMPLETED_NO
            );
        // Do it
        _name=v;
        updateAbsoluteName();
      }
      break;

    default:
        assert(false); // Never get here.
  }; // end switch.
}

char* Contained_impl::version() 
{
  return CORBA::string_dup(_version);
}

void Contained_impl::version(const char* v)
{
  checkReadonly();
  // Empty strings ARE allowed.
  assert(v);
  _version=v;
}

Container_ptr Contained_impl::defined_in() 
{
  assert(_defined_in);
  return _defined_in->_this();
}

char *Contained_impl::absolute_name() 
{
  return CORBA::string_dup(_absolute_name);
}

void Contained_impl::updateAbsoluteName()
{
  assert(_name.in());
  assert(_name.in()[0]);

  string parent="";

  Contained_impl* contained=dynamic_cast<Contained_impl*>(_defined_in);
  if(contained)
  {
    CORBA::String_var containerName =contained->absolute_name();
    parent=containerName.in();
  }
  _absolute_name=(parent+"::"+_name.in()).c_str();
  // ?? If this object is a Container, we should also call updateAbsoluteName()
  // ?? of any contained object.
}

Repository_ptr Contained_impl::containing_repository() 
{
  return Repository_impl::inst()._this();
}

void Contained_impl::move(
  Container_ptr new_container,
  const char*   new_name,
  const char*   new_version
)
{
  assert(new_name);
  assert(new_version);
  DB(5,"Contained_impl::move("<<new_name<<":"<<new_version<<")")
  checkReadonly();
    
  if(CORBA::is_nil(new_container))
      throw CORBA::BAD_PARAM(
        IFELSE_OMNIORB4(omni::BAD_PARAM_TargetIsInvalidContainer,4),
        CORBA::COMPLETED_NO
      );

  PortableServer::Servant servant =NULL;
  try
  {
    PortableServer::POA_var poa=_default_POA();
    servant=poa->reference_to_servant(new_container);
  }
  catch(PortableServer::POA::ObjectNotActive&){servant=NULL;}
  catch(PortableServer::POA::WrongAdapter&){servant=NULL;}
  catch(PortableServer::POA::WrongPolicy&)
  {
    cerr<<"Can't move contained object.\n"
      "POA has wrong policy for reference_to_servant()."<<endl;
    exit(1); // Programming error - so quit.
  }
  Container_impl* newContainer =dynamic_cast<Container_impl*>(servant);

  // the containers must be in the same repository
  // If newContainer is NULL, then we didn't find it in our repository's POA.
  if(!newContainer || !newContainer->canContain(this->def_kind()))
      throw CORBA::BAD_PARAM(
        IFELSE_OMNIORB4(omni::BAD_PARAM_TargetIsInvalidContainer,4),
        CORBA::COMPLETED_NO
      );
      
  // Have we been asked to move the object to the container it started in?
  if(newContainer==_defined_in)
  {
    // Just change the name and version.
    name(new_name); // May throw exception if new_name is already in use.
    version(new_version);
  }
  else
  {
    // Move to the new container.
    Container_impl* oldContainer = _defined_in;
    Identifier_var  oldName      = _name.in();
    int             oldIndex     = _index;
    _defined_in=newContainer;
    _name=""; // Clear, as this object current has no name in newContainer.
    DB(5,"Contained_impl::move(): MOVE "<<oldName.in()<<"->"<<new_name<<")")
    try
    {
      name(new_name); // Throws BAD_PARAM if new_name is already in use.
      newContainer->addContained(this); // May throw BAD_PARAM
    }
    catch(CORBA::BAD_PARAM&)
    {
      // Restore _defined_in & _name.
      _defined_in=oldContainer;
      _name=oldName;
      _index=oldIndex;
      throw;
    }
    version(new_version); // never throws
    oldContainer->removeContained(this);
  }
}

void Contained_impl::uncheckedDestroy()
{
  if(_defined_in)
  {
    _defined_in->removeContained(this);
    _defined_in=NULL;
  }
  if(_id.in())
  {
    Repository_impl::inst().removeId(_id.in());
  }
}

char* Contained_impl::definedInId()
{
  Contained_impl* contained =dynamic_cast<Contained_impl*>(_defined_in);
  if(contained)
      return contained->id();
  else
      return string_dup("");
}

} // end namespace Omniifr
