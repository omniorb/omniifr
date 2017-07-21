//                            Package   : omniIFR
//  ExceptionDef.cc           Created   : 2004/02/22
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
#include "ExceptionDef.h"

#include "Repository.h"
#include "idltype.h"
#include "PersistNode.h"

namespace Omniifr {

TypeCode_ptr ExceptionDef_impl::type()
{
  return Repository_impl::inst()._orb
         ->create_exception_tc(_id.in(),_name.in(),_members.in());
}

StructMemberSeq* ExceptionDef_impl::members()
{
  return _members.copy();
}

void ExceptionDef_impl::members(const StructMemberSeq& v)
{
  checkReadonly();
  // Essentially this method is the same as StructDef_impl::members().
  // See that method for implementation notes.
  StructMemberSeq temp(v); // take a copy (v is const).
  for(ULong i=0; i<temp.length(); i++)
  {
    if(CORBA::is_nil(temp[i].type_def))
        throw CORBA::BAD_PARAM(
          IFELSE_OMNIORB4(omni::BAD_PARAM_InvalidObjectRef,43),
          CORBA::COMPLETED_NO
        );
    try {
        temp[i].type=temp[i].type_def->type();
    }
    catch(...) {
      DB(15,"Caught exception at "<<__FILE__<<":"<<__LINE__)
      throw; // If an exception occurs, just propagate it as-is.
    }
  }

  _members.assign(temp);
}

CORBA::Contained::Description* ExceptionDef_impl::describe()
{
  ExceptionDescription_var desc =new ExceptionDescription();
  desc->name       = name();
  desc->id         = id();
  desc->defined_in = definedInId();
  desc->version    = version();
  desc->type       = type();

  CORBA::Contained::Description_var description =
    new CORBA::Contained::Description();
  description->kind  =   def_kind();
  description->value <<= desc._retn();

  return description._retn();
}

void ExceptionDef_impl::uncheckedDestroy()
{
  _members.clear();
  Contained_impl::uncheckedDestroy();
  Container_impl::uncheckedDestroy();
}

bool ExceptionDef_impl::canContain(DefinitionKind kind)
{
  switch(kind)
  {
    case dk_Struct:
    case dk_Union:
    case dk_Enum:
      return true;
    default:
      return false;
  }
}

void ExceptionDef_impl::reincarnate(const PersistNode& node)
{
  cdrMemoryStream memstr =node.attrCdrStream("state");
  StructMemberSeq m;
  m<<=memstr;
  _members.uncheckedAssign(m); // Skip the checks performed by members(m)
}

void ExceptionDef_impl::output(ostream &os)
{
  outputSelf(os,"ExceptionDef");
  cdrMemoryStream memstr(CORBA::ULong(0),CORBA::Boolean(1)/*clearMemory*/);
  _members.in()>>=memstr;
  PersistNode::outputCdrMemoryStream(os,memstr,"\n state=");
  os<<" ;;\n";
  outputContents(os);
}

} // end namespace Omniifr
