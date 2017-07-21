//                            Package   : omniIFR
//  AttributeDef.cc           Created   : 2004/02/22
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
#include "AttributeDef.h"

#include "idltype.h"
#include "PersistNode.h"
#include "string_to.h"

namespace Omniifr {

TypeCode_ptr AttributeDef_impl::type()
{
  return _type_def.in()->type();
}

IDLType_ptr AttributeDef_impl::type_def()
{
  return IDLType::_duplicate(_type_def.in());
}

void AttributeDef_impl::type_def(IDLType_ptr v)
{
  checkReadonly();
  _type_def.assign(IDLType::_duplicate(v));
}


CORBA::Contained::Description* AttributeDef_impl::describe()
{
  AttributeDescription_var attributedesc =new AttributeDescription();
  attributedesc->name       =name();
  attributedesc->id         =id();
  attributedesc->version    =version();
  attributedesc->type       =type();
  attributedesc->mode       =mode();
  attributedesc->defined_in =definedInId();

  CORBA::Contained::Description_var description =
    new CORBA::Contained::Description();
  description->kind  =   def_kind();
  description->value <<= attributedesc._retn();

  return description._retn();
}

void AttributeDef_impl::uncheckedDestroy()
{
  _type_def.clear();
  Contained_impl::uncheckedDestroy();
}

void AttributeDef_impl::reincarnate(const PersistNode& node)
{
  _mode=(AttributeMode)node.attrLong("mode");
  _type_def.assign(
    string_to_<CORBA::IDLType>(node.attrString("type_def").c_str())
  );
}

void AttributeDef_impl::output(ostream &os)
{
  outputSelf(os,"AttributeDef");
  os<<"\n mode="<<_mode;
  PersistNode::outputIOR(os,_type_def.in(),"\n type_def=");
  os<<" ;;\n";
}

} // end namespace Omniifr
