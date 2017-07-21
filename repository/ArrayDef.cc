//                            Package   : omniIFR
//  ArrayDef.cc               Created   : 2004/02/22
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
#include "ArrayDef.h"
#include "Repository.h"
#include "PersistNode.h"
#include "string_to.h"

namespace Omniifr {

TypeCode_ptr ArrayDef_impl::element_type()
{
  return _element_type_def.in()->type();
}

CORBA::IDLType_ptr ArrayDef_impl::element_type_def()
{
  return _element_type_def.copy();
}

void ArrayDef_impl::element_type_def(CORBA::IDLType_ptr v)
{
  checkReadonly();
  _element_type_def.assign(CORBA::IDLType::_duplicate(v));
}

TypeCode_ptr ArrayDef_impl::type()
{
  TypeCode_var tc =element_type();
  return Repository_impl::inst()._orb->create_array_tc(_length,tc.in());
}

ArrayDef_impl::ArrayDef_impl(ULong length):
  _length(length),
  _element_type_def(this)
{
  Repository_impl::inst().addAnonymous(this);
}

void ArrayDef_impl::uncheckedDestroy()
{
  Repository_impl::inst().removeAnonymous(this);
  _element_type_def.clear();
}

void ArrayDef_impl::reincarnate(const PersistNode& node)
{
  _element_type_def.assign(
    string_to_<CORBA::IDLType>(node.attrString("element_type_def").c_str())
  );
}

void ArrayDef_impl::output(ostream &os)
{
  os<<"ArrayDef"<<PersistNode::_separator;
  outputOid(os);
  os<<"\n length="<<_length;
  PersistNode::outputIOR(os,_element_type_def.in(),"\n element_type_def=");
  os<<" ;;\n";
}

} // end namespace Omniifr
