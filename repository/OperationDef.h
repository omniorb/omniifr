//                            Package   : omniIFR
//  OperationDef.h            Created   : 2004/02/22
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
#ifndef OMNIIFR__OPERATIONDEF_H
#define OMNIIFR__OPERATIONDEF_H

#include "Contained.h"
#include "Dependency.h"

namespace Omniifr {

class IDLType_impl;
class Container_impl;
class Repository_impl;

class OperationDef_impl :
  public virtual POA_CORBA::OperationDef,
  public         Contained_impl
{
public: // CORBA attributes
  TypeCode_ptr       result();
  IDLType_ptr        result_def();
  void               result_def(IDLType_ptr v);
  ParDescriptionSeq* params();
  void               params(const ParDescriptionSeq& v);
  OperationMode      mode();
  void               mode(OperationMode v);
  ContextIdSeq*      contexts();
  void               contexts(const ContextIdSeq& v);
  ExceptionDefSeq*   exceptions();
  void               exceptions(const ExceptionDefSeq& v);
public: // CORBA interface methods (from IRObject)
  DefinitionKind def_kind(){return CORBA::dk_Operation;}
public: // CORBA interface methods (from Contained)
  CORBA::Contained::Description* describe();

private:
  TypeCode_var                   _result; ///< set from _result_def
  Dependency1<IDLType>           _result_def;
  Dependency3<ParDescriptionSeq> _params;
  OperationMode                  _mode;
  ContextIdSeq                   _contexts;
  Dependency2<ExceptionDefSeq>   _exceptions;
  ExcDescriptionSeq              _excDescriptionSeq; ///< set from _exceptions

public:
  OperationDef_impl():
    _result(),
    _result_def(this),
    _params(this),
    _mode(CORBA::OP_NORMAL),
    _contexts(),
    _exceptions(this),
    _excDescriptionSeq()
  {}
  virtual ~OperationDef_impl() {}
  void uncheckedDestroy();
  void reincarnate(const PersistNode& node);
  void output(ostream &os);

private:
  /** Tests for a violation of the rules on oneway operations. If a violation
   * is found, then raises BAD_PARAM(31), else does nothing.
   */
  void checkOneway(
    const OperationMode      mode,
    const ParDescriptionSeq& params,
    IDLType_ptr              result_def
  ) const;
};

} // end namespace Omniifr

#endif // OMNIIFR__OPERATIONDEF_H
