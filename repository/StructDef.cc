//                            Package   : omniIFR
//  StructDef.cc              Created   : 2004/02/22
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
#include "StructDef.h"

#include "Repository.h"
#include "PersistNode.h"

namespace Omniifr {

StructMemberSeq* StructDef_impl::members()
{
  return _members.copy();
}

void StructDef_impl::members(const StructMemberSeq& v)
{
  checkReadonly();
  // Spec: "When setting the members attribute, the type member of the
  // StructMember structure should be set to TC_void."
  // I interpret this as meaning that the type member is set by the setter
  // method (here). The word 'should' implies a recommendation rather than an 
  // imperative - I choose to not throw an exception if type is set, only to
  // ignore its value.
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

TypeCode_ptr StructDef_impl::type()
{
  return Repository_impl::inst()._orb
         ->create_struct_tc(_id.in(),_name.in(),_members.in());
}

void StructDef_impl::uncheckedDestroy()
{
  _members.clear();
  Contained_impl::uncheckedDestroy();
  Container_impl::uncheckedDestroy();
}

bool StructDef_impl::canContain(DefinitionKind kind)
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

void StructDef_impl::reincarnate(const PersistNode& node)
{
  cdrMemoryStream memstr =node.attrCdrStream("state");
  StructMemberSeq m;
  m<<=memstr;
  _members.uncheckedAssign(m); // Skip the checks performed by members(m)
}

void StructDef_impl::output(ostream &os)
{
  outputSelf(os,"StructDef");
  cdrMemoryStream memstr(CORBA::ULong(0),CORBA::Boolean(1)/*clearMemory*/);
  _members.in()>>=memstr;
  PersistNode::outputCdrMemoryStream(os,memstr,"\n state=");
  os<<" ;;\n";
  outputContents(os);
}

} // end namespace Omniifr
