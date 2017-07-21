//                            Package   : omniIFR
//  AliasDef.cc               Created   : 2004/02/22
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
#include "AliasDef.h"
#include "Repository.h"
#include "PersistNode.h"
#include "string_to.h"

namespace Omniifr {

IDLType_ptr AliasDef_impl::original_type_def()
{
  return _original_type_def.copy();
}

void AliasDef_impl::original_type_def(IDLType_ptr v)
{
  checkReadonly();
  _original_type_def.assign(CORBA::IDLType::_duplicate(v));
}

TypeCode_ptr AliasDef_impl::type() 
{
  TypeCode_var tc=_original_type_def.in()->type();
  return Repository_impl::inst()._orb
         ->create_alias_tc(_id.in(),_name.in(),tc.in());
}

void AliasDef_impl::uncheckedDestroy()
{
  _original_type_def.clear();
  Contained_impl::uncheckedDestroy();
}

void AliasDef_impl::reincarnate(const PersistNode& node)
{
  _original_type_def.assign(
    string_to_<CORBA::IDLType>(node.attrString("original_type_def").c_str())
  );
}

void AliasDef_impl::output(ostream &os)
{
  outputSelf(os,"AliasDef");
  PersistNode::outputIOR(os,_original_type_def.in(),"\n original_type_def=");
  os<<" ;;\n";
}

} // end namespace Omniifr
