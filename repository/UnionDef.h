//                            Package   : omniIFR
//  UnionDef.h                Created   : 2004/02/22
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
#ifndef OMNIIFR__UNIONDEF_H
#define OMNIIFR__UNIONDEF_H

#include "TypedefDef.h"
#include "Container.h"
#include "Dependency.h"

namespace Omniifr {

class UnionDef_impl :
  public virtual POA_CORBA::UnionDef,
  public         TypedefDef_impl,
  public         Container_impl
{
public: // CORBA attributes
  TypeCode_ptr    discriminator_type();
  IDLType_ptr     discriminator_type_def();
  void            discriminator_type_def(IDLType_ptr v);
  UnionMemberSeq* members();
  void            members(const UnionMemberSeq& v);
public: // CORBA interface methods (from IRObject)
  DefinitionKind def_kind(){return dk_Union;}
public: // CORBA attributes (from IDLType)
  TypeCode_ptr type();

private:    
  Dependency1<CORBA::IDLType> _discriminator_type_def;
  Dependency3<UnionMemberSeq> _members;
public:
  UnionDef_impl():_discriminator_type_def(this),_members(this){}
  virtual ~UnionDef_impl() {}
  void uncheckedDestroy();
  bool canContain(DefinitionKind kind);
  void reincarnate(const PersistNode& node);
  void output(ostream &os);
};

} // end namespace Omniifr

#endif // OMNIIFR__UNIONDEF_H
