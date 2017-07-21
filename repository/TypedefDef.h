//                            Package   : omniIFR
//  TypedefDef.h              Created   : 2004/02/22
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
#ifndef OMNIIFR__TYPEDEFDEF_H
#define OMNIIFR__TYPEDEFDEF_H

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifdef HAVE_IOSTREAM
#  include <iostream>
#else
#  include <iostream.h>
#endif

#ifdef HAVE_STD_IOSTREAM
using namespace std;
#endif

#include "idltype.h"
#include "Contained.h"

/** Used in switch statements that need to have a case for typedefs. Rather
 * than listing all of the typedef types, just use this macro instead.
 */
#define CASE_TYPEDEF \
  case dk_Typedef: \
    cerr<<"WARNING: kind==dk_Typedef. This should be impossible."<<endl; \
  case dk_Union: \
  case dk_Alias: \
  case dk_Enum: \
  case dk_Native: \
  case dk_Struct:

namespace Omniifr {

class TypedefDef_impl :
  public virtual POA_CORBA::TypedefDef,
  public         Contained_impl,
  public         IDLType_impl
{
public: // CORBA interface methods (from Contained)
  CORBA::Contained::Description* describe();

public:
  TypedefDef_impl() {}
  virtual ~TypedefDef_impl() {}
};

} // end namespace Omniifr

#endif // OMNIIFR__TYPEDEFDEF_H
