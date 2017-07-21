//                            Package   : omniIFR
//  PrimitiveDef.h            Created   : 2004/02/22
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
#ifndef OMNIIFR__PRIMITIVEDEF_H
#define OMNIIFR__PRIMITIVEDEF_H

#include "idltype.h"

namespace Omniifr {

class PrimitiveDef_impl :
  public virtual POA_CORBA::PrimitiveDef,
  public         IDLType_impl
{
public: // CORBA attributes
  CORBA::PrimitiveKind kind(){return _kind;}
public: // CORBA interface methods (from IRObject)
  CORBA::DefinitionKind def_kind(){return CORBA::dk_Primitive;}
  /** Always throws BAD_INV_ORDER/ObjectIndestructible. */
  void destroy();

public: // CORBA attributes (from IDLType)
  CORBA::TypeCode_ptr type();

private:
  CORBA::PrimitiveKind _kind;    
public:
  PrimitiveDef_impl(CORBA::PrimitiveKind kind);
  virtual ~PrimitiveDef_impl(){}
  void uncheckedDestroy(){}
  void output(ostream &os){} ///< Primitive types do not need to be stored.
};

} // end namespace Omniifr

#endif // OMNIIFR__PRIMITIVEDEF_H
