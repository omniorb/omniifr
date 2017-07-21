//                            Package   : omniIFR
//  ModuleDef.h               Created   : 2004/02/22
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
#ifndef OMNIIFR__MODULEDEF_H
#define OMNIIFR__MODULEDEF_H

#include "Container.h"
#include "Contained.h"

namespace Omniifr {

class ModuleDef_impl :
  public virtual POA_CORBA::ModuleDef,
  public         Container_impl,
  public         Contained_impl
{
public: // CORBA interface methods (from IRObject)
  CORBA::DefinitionKind def_kind(){return CORBA::dk_Module;}
public: // CORBA interface methods (from Contained)
  CORBA::Contained::Description* describe();

public:
  ModuleDef_impl() {}
  virtual ~ModuleDef_impl() {};
  void uncheckedDestroy();
  bool canContain(DefinitionKind kind);
  void output(ostream &os);
};

} // end namespace Omniifr

#endif // OMNIIFR__MODULEDEF_H
