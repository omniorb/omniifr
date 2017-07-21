//                            Package   : omniIFR
//  UnionDef.cc               Created   : 2004/02/22
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
#include "UnionDef.h"

#include "Repository.h"
#include "PersistNode.h"
#include "string_to.h"

#include <string.h>

namespace Omniifr {

TypeCode_ptr UnionDef_impl::discriminator_type()
{
  return _discriminator_type_def.in()->type();
}

IDLType_ptr UnionDef_impl::discriminator_type_def()
{
  return _discriminator_type_def.copy();
}

void UnionDef_impl::discriminator_type_def(IDLType_ptr v)
{
  checkReadonly();
  TypeCode_var tc    =v->type();
  TCKind       kind  =tc->kind();
  // Strip out aliases.
  while(kind==tk_alias)
  {
    tc=tc->content_type();
    kind=tc->kind();
  }

  switch(kind)
  {
    case tk_short: // <integer_type>
    case tk_ushort:
    case tk_long:
    case tk_ulong:
#ifdef HAS_LongLong
    case tk_longlong:
    case tk_ulonglong:
#endif
    case tk_char: // <char_type>
    case tk_wchar: // <wide_char_type>
    case tk_boolean: // <boolean_type>
    case tk_enum:
      {
        // If discriminator type has changed, then erase _members here,
        // to protect against type incompatability.
        if(!is_nil(_discriminator_type_def.in()))
        {
          TypeCode_var oldTc =discriminator_type();
          if(!tc->equivalent(oldTc.in()))
              _members.assign(UnionMemberSeq()); // clear _members.
        }
        _discriminator_type_def.assign(CORBA::IDLType::_duplicate(v));
      }
      break;

    default:
      throw CORBA::BAD_PARAM(
        IFELSE_OMNIORB4(omni::BAD_PARAM_IllegitimateDiscriminatorType,20),
        CORBA::COMPLETED_NO
      );
  }
}

UnionMemberSeq* UnionDef_impl::members()
{
  return _members.copy();
}

void UnionDef_impl::members(const UnionMemberSeq& v)
{
  checkReadonly();
  UnionMemberSeq temp(v); // take a copy (v is const).
  TypeCode_var discriminatorType =discriminator_type();
  
  // Note: These checks are similar to Dependency3::check(), but more
  // extensive.
  
  for(ULong i=0; i<temp.length(); ++i)
  {
    IdentifierUtil::checkInvalid(temp[i].name);
    if(CORBA::is_nil(temp[i].type_def))
        throw CORBA::BAD_PARAM(
          IFELSE_OMNIORB4(omni::BAD_PARAM_InvalidObjectRef,43),
          CORBA::COMPLETED_NO
        );

    // Set type to match type_def. (Ignore type's current value.)
    try
    {
      temp[i].type=temp[i].type_def->type();
    }
    catch(...)
    {
      DB(15,"Caught exception at "<<__FILE__<<":"<<__LINE__)
      throw; // If an exception occurs, just propagate it as-is.
    }

    // The default label (Octet with the value zero) is always allowed.
    CORBA::Octet oct;
    bool isDefaultLabel=((temp[i].label>>=CORBA::Any::to_octet(oct)) && oct==0);
    if(!isDefaultLabel)
    {
      // Check that label's type matches discriminator_type.
      TypeCode_var labelType =temp[i].label.type();
      if(!labelType->equivalent(discriminatorType.in()))
          throw CORBA::BAD_PARAM(
            IFELSE_OMNIORB4(omni::BAD_PARAM_IncompatibleDiscriminatorType,19),
            CORBA::COMPLETED_NO
          );

      // Check that the label is unique.
      DynamicAny::DynAny_var dai =
        Repository_impl::inst()._DynAnyFactory->create_dyn_any(temp[i].label);
      bool uniqueLabel =true;
      for(ULong j=0; uniqueLabel && j<i; ++j)
      {
        DynamicAny::DynAny_var daj =
          Repository_impl::inst()._DynAnyFactory->create_dyn_any(temp[j].label);
        if(dai->equal(daj))
            uniqueLabel=false;
        daj->destroy();
      }
      dai->destroy();
      if(!uniqueLabel)
          throw CORBA::BAD_PARAM(
            IFELSE_OMNIORB4(omni::BAD_PARAM_DuplicateLabelValue,18),
            CORBA::COMPLETED_NO
          );
    }

    // Check that name/type is legal.
    // name must be unique, UNLESS it AND ITS TYPE are the same as the previous
    // member.
    bool match =true;
    for(Long k=i-1; k>=0; --k)
    {
      switch(IdentifierUtil::compare(temp[i].name,temp[k].name))
      {
        case IdentifierUtil::equalMatch:
            if(! (match && temp[i].type->equal(temp[k].type)) )
                throw CORBA::BAD_PARAM(); // ?? Minor code?
            break;

        case IdentifierUtil::equivalentMatch:
            throw CORBA::BAD_PARAM(
              IFELSE_OMNIORB4(omni::BAD_PARAM_InvalidMemberName,17),
              CORBA::COMPLETED_NO
            );

        case IdentifierUtil::noMatch:
            match=false;
            break;

        default:
            assert(false); // Never get here.
      }; // end switch.
    }
  }
  
  // Do it.
  _members.uncheckedAssign(temp);
}

TypeCode_ptr UnionDef_impl::type()
{
  TypeCode_var tc =discriminator_type();
  return Repository_impl::inst()._orb
    ->create_union_tc(_id.in(),_name.in(),tc.in(),_members.in());
}

void UnionDef_impl::uncheckedDestroy()
{
  _members.clear();
  _discriminator_type_def.clear();
  Contained_impl::uncheckedDestroy();
  Container_impl::uncheckedDestroy();
}

bool UnionDef_impl::canContain(DefinitionKind kind)
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

void UnionDef_impl::reincarnate(const PersistNode& node)
{
  _discriminator_type_def.assign(
   string_to_<CORBA::IDLType>(node.attrString("discriminator_type_def").c_str())
  );

  cdrMemoryStream memstr =node.attrCdrStream("state");
  UnionMemberSeq m;
  m<<=memstr;
  _members.uncheckedAssign(m); // Skip the checks performed by members(m)
}

void UnionDef_impl::output(ostream &os)
{
  outputSelf(os,"UnionDef");
  PersistNode::outputIOR(os,
    _discriminator_type_def.in(),
    "\n discriminator_type_def="
  );
  cdrMemoryStream memstr(CORBA::ULong(0),CORBA::Boolean(1)/*clearMemory*/);
  _members.in()>>=memstr;
  PersistNode::outputCdrMemoryStream(os,memstr,"\n state=");
  os<<" ;;\n";
  outputContents(os);
}

} // end namespace Omniifr
