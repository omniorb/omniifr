//                            Package   : omniIFR
//  OperationDef.cc           Created   : 2004/02/22
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
#include "OperationDef.h"

#include "idltype.h"
#include "Container.h"
#include "ExceptionDef.h"
#include "PersistNode.h"
#include "string_to.h"

namespace Omniifr {

TypeCode_ptr
OperationDef_impl::result()
{
  return TypeCode::_duplicate(_result.in());
}

IDLType_ptr
OperationDef_impl::result_def()
{
  return IDLType::_duplicate(_result_def.in());
}

void
OperationDef_impl::result_def(IDLType_ptr v)
{
  checkReadonly();
  checkOneway(_mode,_params.in(),v); // throws BAD_PARAM if check fails
  _result_def.assign(IDLType::_duplicate(v));
  _result=v->type();
}

ParDescriptionSeq* OperationDef_impl::params()
{
  return _params.copy();
}

void OperationDef_impl::params(const ParDescriptionSeq& v)
{
  checkReadonly();
  checkOneway(_mode,v,_result_def.in()); // throws BAD_PARAM if check fails
  _params.assign(v);
}

OperationMode OperationDef_impl::mode()
{
  return _mode;
}

void OperationDef_impl::mode(OperationMode v)
{
  checkReadonly();
  if(v!=OP_NORMAL && v!=OP_ONEWAY)
      throw CORBA::BAD_PARAM(
        IFELSE_OMNIORB4(omni::BAD_PARAM_EnumValueOutOfRange,25),
        CORBA::COMPLETED_NO
      );
  checkOneway(v,_params.in(),_result_def.in());
  _mode=v;
}

ContextIdSeq* OperationDef_impl::contexts()
{
  return new ContextIdSeq(_contexts);
}

void OperationDef_impl::contexts(const ContextIdSeq& v)
{
  checkReadonly();
  _contexts=v;
}

ExceptionDefSeq* OperationDef_impl::exceptions()
{
  return _exceptions.copy();
}

void OperationDef_impl::exceptions(const ExceptionDefSeq& v)
{
  checkReadonly();
  // Construct a new ExcDescriptionSeq, ready to assign to _excDescriptionSeq.
  // (Checks for problems before we do anything irrevocable.)
  ExcDescriptionSeq eds;
  eds.length(v.length());
  for(CORBA::ULong i=0; i<v.length(); i++)
  {
    CORBA::Contained::Description_var description =v[i]->describe();
    const ExceptionDescription* exceptionDescription;
    Boolean ok( description->value >>= exceptionDescription );
    if(!ok)
        throw CORBA::BAD_PARAM(
          IFELSE_OMNIORB4(omni::BAD_PARAM_EnumValueOutOfRange,25),
          CORBA::COMPLETED_NO
        );
    eds[i]=*exceptionDescription; // take a copy
    // No need to delete exceptionDescription - it's still owned by the Any.
  }
  // Perform the following two assignments in the correct order to ensure that
  // both _exceptions & _excDescriptionSeq remain untouched if an exception
  // occurs.
  _exceptions.assign(v); // may throw BAD_PARAM
  _excDescriptionSeq=eds; // does not throw.
}

CORBA::Contained::Description* OperationDef_impl::describe()
{
  OperationDescription_var desc =new OperationDescription();
  desc->name       = name();
  desc->id         = id();
  desc->version    = version();
  desc->result     = result();
  desc->mode       = _mode;
  desc->contexts   = _contexts;
  desc->parameters = _params.in();
  desc->exceptions = _excDescriptionSeq;
  desc->defined_in = definedInId();

  CORBA::Contained::Description_var description =
    new CORBA::Contained::Description();
  description->kind  =   def_kind();
  description->value <<= desc._retn();

  return description._retn();
}

void OperationDef_impl::uncheckedDestroy()
{
  _result_def.clear();
  _params.clear();
  _exceptions.clear();
  Contained_impl::uncheckedDestroy(); // superclass
}

void OperationDef_impl::reincarnate(const PersistNode& node)
{
  _result_def.assign(
    string_to_<CORBA::IDLType>(node.attrString("result_def").c_str())
  );
  // Read state.
  // Order must match that in OperationDef_impl::output():
  cdrMemoryStream memstr =node.attrCdrStream("state");
  _result=TypeCode::unmarshalTypeCode(memstr);  // _result
  ParDescriptionSeq       p;                    // _params
                          p<<=memstr;
  _params.uncheckedAssign(p);
  _mode<<=memstr;                               // _mode
  _contexts<<=memstr;                           // _contexts
  ExceptionDefSeq    e;                         // _exceptions
                     e<<=memstr;
  _exceptions.assign(e);
  _excDescriptionSeq<<=memstr;                  // _excDescriptionSeq
}

void OperationDef_impl::output(ostream &os)
{
  outputSelf(os,"OperationDef");
  PersistNode::outputIOR(os,_result_def.in(),"\n result_def=");
  cdrMemoryStream memstr(CORBA::ULong(0),CORBA::Boolean(1)/*clearMemory*/);
  TypeCode::marshalTypeCode(_result.in(),memstr);
  _params.in()>>=memstr;
  _mode>>=memstr;
  _contexts>>=memstr;
  _exceptions.in()>>=memstr;
  _excDescriptionSeq>>=memstr;
  PersistNode::outputCdrMemoryStream(os,memstr,"\n state=");
  os<<" ;;\n";
}

void OperationDef_impl::checkOneway(
  const OperationMode      mode,
  const ParDescriptionSeq& params,
  IDLType_ptr              result_def
) const
{
  // Only worry about oneway operations.
  // If result_def is not yet set, then don't perform the check. This should
  // only happen during a call to InterfaceDef_impl::create_operation().
  if(mode==OP_ONEWAY && !CORBA::is_nil(result_def))
  {
    // Return type must be void.
    TypeCode_var resultTc =result_def->type();
    if(resultTc->kind()!=tk_void)
        throw CORBA::BAD_PARAM(31,CORBA::COMPLETED_NO);

    // Parameters' modes must be PARAM_IN.
    for(ULong i=0; i<params.length(); ++i)
    {
      if(params[i].mode!=PARAM_IN)
        throw CORBA::BAD_PARAM(31,CORBA::COMPLETED_NO);
    }
  }
}

} // end namespace Omniifr
