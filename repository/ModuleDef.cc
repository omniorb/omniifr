//                            Package   : omniIFR
//  ModuleDef.cc              Created   : 2004/02/22
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
#include "ModuleDef.h"

#include "TypedefDef.h" // for CASE_TYPEDEF macro

namespace Omniifr {

void ModuleDef_impl::uncheckedDestroy()
{
  Contained_impl::uncheckedDestroy();
  Container_impl::uncheckedDestroy();
}

Contained::Description* ModuleDef_impl::describe()
{
  ModuleDescription_var moduledesc =new ModuleDescription();
  moduledesc->name       = name();
  moduledesc->id         = id();
  moduledesc->version    = version();
  moduledesc->defined_in = definedInId();

  CORBA::Contained::Description_var description =
    new CORBA::Contained::Description();
  description->kind  =   def_kind();
  description->value <<= moduledesc._retn();

  return description._retn();
}

bool ModuleDef_impl::canContain(DefinitionKind kind)
{
  switch(kind)
  {
    CASE_TYPEDEF
    case dk_Constant:
    case dk_Exception:
    case dk_Interface:
    case dk_Value:
    case dk_ValueBox:
    case dk_Module:
      return true;
    default:
      return false;
  }
}

void ModuleDef_impl::output(ostream &os)
{
  outputSelf(os,"ModuleDef");
  os<<" ;;\n";
  outputContents(os);
}

} // end namespace Omniifr
