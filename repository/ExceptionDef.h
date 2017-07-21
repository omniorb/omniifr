//                            Package   : omniIFR
//  ExceptionDef.h            Created   : 2004/02/22
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
#ifndef OMNIIFR_EXCEPTION_H
#define OMNIIFR_EXCEPTION_H

#include "Contained.h"
#include "Container.h"
#include "Dependency.h"

namespace Omniifr {

class Repository_impl;

class ExceptionDef_impl :
  public virtual POA_CORBA::ExceptionDef,
  public         Contained_impl,
  public         Container_impl
{
public: // CORBA attributes
  TypeCode_ptr     type();
  StructMemberSeq* members();
  void             members(const StructMemberSeq&);
public: // CORBA interface methods(from IRObject)
  DefinitionKind def_kind(){return CORBA::dk_Exception;}
public: // CORBA interface methods (from Contained)
  CORBA::Contained::Description* describe();

private:
  Dependency3<StructMemberSeq> _members;
public:
  ExceptionDef_impl():_members(this){}
  virtual ~ExceptionDef_impl(){}
  void uncheckedDestroy();
  bool canContain(DefinitionKind kind);
  void reincarnate(const PersistNode& node);
  void output(ostream &os);
};

} // end namespace Omniifr

#endif // OMNIIFR_EXCEPTION_H
