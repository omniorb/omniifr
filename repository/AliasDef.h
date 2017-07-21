//                            Package   : omniIFR
//  AliasDef.h                Created   : 2004/02/22
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
#ifndef OMNIIFR__ALIASDEF_H
#define OMNIIFR__ALIASDEF_H

#include "TypedefDef.h"
#include "Dependency.h"

namespace Omniifr {

class AliasDef_impl :
  public virtual POA_CORBA::AliasDef,
  public         TypedefDef_impl
{
public: // CORBA attributes
  IDLType_ptr original_type_def();
  void        original_type_def(IDLType_ptr v);
public: // CORBA interface methods (from IRObject)
  DefinitionKind def_kind(){return dk_Alias;}
public: // CORBA attributes (from IDLType)
  TypeCode_ptr type();

private:
  Dependency1<CORBA::IDLType> _original_type_def;
public:
  AliasDef_impl():_original_type_def(this){}
  virtual ~AliasDef_impl() {}
  void uncheckedDestroy();
  void reincarnate(const PersistNode& node);
  void output(ostream &os);
};

} // end namespace Omniifr

#endif // OMNIIFR__ALIASDEF_H
