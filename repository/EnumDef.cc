//                            Package   : omniIFR
//  EnumDef.cc                Created   : 2004/02/22
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
#include "EnumDef.h"
#include "Repository.h"
#include "IdentifierUtil.h"
#include "PersistNode.h"

namespace Omniifr {

EnumMemberSeq* EnumDef_impl::members()
{
  return new EnumMemberSeq(_members);
}

void EnumDef_impl::members(const EnumMemberSeq& v)
{
  checkReadonly();
  // Check for invalid v.
  for(ULong i=0; i<v.length(); i++)
  {
    IdentifierUtil::checkInvalid(v[i]); // throws BAD_PARAM if v[i] is invalid.
    for(ULong j=0; j<i; j++)
    {
      if(0==strcasecmp(v[i],v[j])) // case insensitive
          throw CORBA::BAD_PARAM(
            IFELSE_OMNIORB4(omni::BAD_PARAM_ValueFactoryFailure,1),
            CORBA::COMPLETED_NO
          );
    }
  }
  // Do the work.
  _members=v;
}

TypeCode_ptr EnumDef_impl::type()
{
  return Repository_impl::inst()._orb
         ->create_enum_tc(_id.in(),_name.in(),_members);
}

void EnumDef_impl::uncheckedDestroy()
{
  Contained_impl::uncheckedDestroy();
}

void EnumDef_impl::reincarnate(const PersistNode& node)
{
  cdrMemoryStream memstr =node.attrCdrStream("state");
  _members<<=memstr;
}

void EnumDef_impl::output(ostream &os)
{
  outputSelf(os,"EnumDef");
  cdrMemoryStream memstr(CORBA::ULong(0),CORBA::Boolean(1)/*clearMemory*/);
  _members>>=memstr;
  PersistNode::outputCdrMemoryStream(os,memstr,"\n state=");
  os<<" ;;\n";
}

} // end namespace Omniifr
