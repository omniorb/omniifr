//                            Package   : omniIFR
//  ConstantDef.cc            Created   : 2004/02/22
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
#include "ConstantDef.h"

#include "idltype.h"
#include "Container.h"
#include "PersistNode.h"
#include "string_to.h"

#include <stdio.h> // sprintf

namespace Omniifr {

TypeCode_ptr ConstantDef_impl::type()
{
  return _type_def.in()->type();
}

IDLType_ptr ConstantDef_impl::type_def()
{
  return IDLType::_duplicate(_type_def.in());
}

void ConstantDef_impl::type_def(IDLType_ptr v)
{
  checkReadonly();
  TypeCode_var vTc  =v->type();
  TCKind       kind =vTc->kind();
  // Strip out aliases. <scoped_name>
  while(kind==tk_alias)
  {
    vTc=vTc->content_type();
    kind=vTc->kind();
  }

  switch(kind)
  {
    case tk_short:   // <integer_type>
    case tk_ushort: 
    case tk_long: 
    case tk_ulong: 
#ifdef HAS_LongLong
    case tk_longlong: 
    case tk_ulonglong: 
#endif
    case tk_char:    // <char_type>
    case tk_wchar:   // <wide_char_type>
    case tk_boolean: // <boolean_type>
    case tk_float:   // <floating_pt_type>
    case tk_double: 
#ifdef HAS_LongDouble
    case tk_longdouble: 
#endif
    case tk_string:  // <string_type>
    case tk_wstring: // <wide_string_type>
    case tk_fixed:   // <fixed_pt_const_type>
    case tk_octet:   // <octet_type>
      {
        TypeCode_var valueTc=_value.type();
        _type_def.assign(IDLType::_duplicate(v)); // May throw...
        // Erase _value here, unless its TypeCode matches 'v'.
        if(!vTc->equivalent(valueTc))
           _value=Any();
      }
      break;

    default:
      throw CORBA::BAD_PARAM(); // ?? Minor code
  }
}

CORBA::Any* ConstantDef_impl::value()
{
  return new CORBA::Any(_value);
}

void ConstantDef_impl::value(const Any& v)
{
  checkReadonly();
  CORBA::TypeCode_var thisTc =this->type();
  CORBA::TypeCode_var vTc    =v.type();
  if(thisTc->equivalent(vTc.in()))
  {
    _value=v;
  }
  else
  {
    throw CORBA::BAD_PARAM( // Mico throws NO_PERMISSION here.
      IFELSE_OMNIORB4(omni::BAD_PARAM_TargetIsInvalidContainer,4),
      CORBA::COMPLETED_NO
    );
  }
}

Contained::Description* ConstantDef_impl::describe()
{
  ConstantDescription_var constantdesc =new ConstantDescription();
  constantdesc->name       =name();
  constantdesc->id         =id();
  constantdesc->version    =version();
  constantdesc->type       =type();
  constantdesc->value      =_value;
  constantdesc->defined_in =definedInId();

  CORBA::Contained::Description_var description =
    new CORBA::Contained::Description();
  description->kind  =   def_kind();
  description->value <<= constantdesc._retn();

  return description._retn();
}

void ConstantDef_impl::uncheckedDestroy()
{
  _type_def.clear();
  Contained_impl::uncheckedDestroy(); // superclass
}

void ConstantDef_impl::reincarnate(const PersistNode& node)
{
  _type_def.assign(
    string_to_<IDLType>(node.attrString("type_def").c_str())
  );
  cdrMemoryStream memstr =node.attrCdrStream("value");
  _value<<=memstr;
}

void ConstantDef_impl::output(ostream &os)
{
  outputSelf(os,"ConstantDef");
  PersistNode::outputIOR(os,_type_def.in(),"\n type_def=");
  cdrMemoryStream memstr(CORBA::ULong(0),CORBA::Boolean(1)/*clearMemory*/);
  _value>>=memstr;
  PersistNode::outputCdrMemoryStream(os,memstr,"\n value=");
  os<<" ;;\n";
}

} // end namespace Omniifr
