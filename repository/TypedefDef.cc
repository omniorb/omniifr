//                            Package   : omniIFR
//  TypedefDef.cc             Created   : 2004/02/22
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
#include "TypedefDef.h"

namespace Omniifr {

Contained::Description* TypedefDef_impl::describe()
{
  TypeDescription_var typedefdesc = new TypeDescription();
  typedefdesc->name       =name();
  typedefdesc->id         =id();
  typedefdesc->defined_in =definedInId();
  typedefdesc->version    =version();
  typedefdesc->type       =type();

  CORBA::Contained::Description_var description =
    new CORBA::Contained::Description();
  description->kind = this->def_kind();
  description->value <<= typedefdesc._retn();

  return description._retn();
}

} // end namespace Omniifr
