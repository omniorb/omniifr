//                            Package   : omniIFR
//  WstringDef.h              Created   : 2004/02/22
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
#ifndef OMNIIFR__WSTRINGDEF_H
#define OMNIIFR__WSTRINGDEF_H

#include "idltype.h"

namespace Omniifr {

class WstringDef_impl :
  public virtual POA_CORBA::WstringDef,
  public         IDLType_impl
{
public: // CORBA attributes
  CORBA::ULong bound()               {return _bound;}
  void         bound(CORBA::ULong v) {checkReadonly(); _bound=v;}
public: // CORBA interface methods (from IRObject)
  CORBA::DefinitionKind def_kind()   {return CORBA::dk_String;}
public: // CORBA attributes (from IDLType)
  CORBA::TypeCode_ptr type();

private:
  CORBA::ULong _bound;
public:
  WstringDef_impl(CORBA::ULong bound);
  virtual ~WstringDef_impl() {}
  void uncheckedDestroy();
  void output(ostream &os);
};

} // end namespace Omniifr

#endif // OMNIIFR__WSTRINGDEF_H
