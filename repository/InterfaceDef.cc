//                            Package   : omniIFR
//  InterfaceDef.cc           Created   : 2004/02/22
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
#include "InterfaceDef.h"

#include "TypedefDef.h" // for CASE_TYPEDEF macro

#include "AttributeDef.h"
#include "OperationDef.h"
#include "Repository.h"

#include "Creche.h"
#include "IdentifierUtil.h"
#include "PersistNode.h"

#include <algorithm>
#include <assert.h>
#include <string.h>

namespace Omniifr {


InterfaceDefSeq* InterfaceDef_impl::base_interfaces()
{
  return _base_interfaces.copy();
}


void InterfaceDef_impl::base_interfaces(const InterfaceDefSeq& v)
{
  checkReadonly();
  // Raise BAD_PARAM if the name of any object contained by this object
  // conflicts with the name of any object contained by any of the classes
  // in v.

  NoCaseStrSet vNameSet =inheritedNameSet(v);

  NoCaseStrSet myNameSet;
  {
    // ASSERT: There should never be a name clash within a container.
    ContainedSeq_var myAttributes=contents(dk_Attribute,1);
    ContainedSeq_var myOperations=contents(dk_Operation,1);
    assert( nameSetOk(myAttributes.in(),myNameSet) );
    assert( nameSetOk(myOperations.in(),myNameSet) );
  }

  NoCaseStrSet conflictSet;
  set_intersection(
    vNameSet.begin(), vNameSet.end(),
    myNameSet.begin(),myNameSet.end(), inserter(conflictSet,conflictSet.end())
  );
  
  // conflictSet contains conflicts between this->contents and inherited names.
  if(!conflictSet.empty())
  {
    string badNames="";
    for(NoCaseStrSet::const_iterator i=conflictSet.begin();
        i!=conflictSet.end();
        ++i)
    {
      badNames+=" \""+(*i)+"\"";
    }
    DB(1,"BAD_PARAM_InheritedNameClash. InterfaceDef \""<<_name<< \
      "\" base_interfaces() rejected duplicate names:"<<badNames.c_str()<<".")
    throw CORBA::BAD_PARAM(
      IFELSE_OMNIORB4(omni::BAD_PARAM_InheritedNameClash,5),
      CORBA::COMPLETED_NO
    );
  }
  
  // Procede:
  _base_interfaces.assign(v); // May raise BAD_PARAM.
}


Boolean InterfaceDef_impl::is_a(const char* interface_id)
{
  assert(interface_id);

  if(strcmp(interface_id,"IDL:omg.org/CORBA/Object:1.0")==0)
      return true;

  if(strcmp(interface_id,_id.in())==0) // itself
      return true;

  const InterfaceDefSeq& bis =_base_interfaces.in();
  for(ULong i=0; i<bis.length(); i++)
  {
    assert(!CORBA::is_nil(bis[i]));
    if(bis[i]->is_a(interface_id))
        return true;
  }
  return false;
}


CORBA::InterfaceDef::FullInterfaceDescription*
InterfaceDef_impl::describe_interface()
{
  CORBA::InterfaceDef::FullInterfaceDescription_var result =
      new CORBA::InterfaceDef::FullInterfaceDescription();
  result->name            =name();
  result->id              =id();
  result->defined_in      =definedInId();
  result->version         =version();

  ContainedSeq_var containedseq =contents(dk_Operation,false);
  result->operations.length(containedseq->length());
  for(ULong i=0; i < containedseq->length(); i++ )
  {
    CORBA::Contained::Description_var description=containedseq[i]->describe();
    const OperationDescription* operation;
    description->value >>= operation;   // Any still owns memory
    result->operations[i] = *operation; // deep copy
  }

  containedseq=contents(dk_Attribute,false);
  result->attributes.length(containedseq->length());
  for(ULong j=0; j<containedseq->length(); j++)
  {
    CORBA::Contained::Description_var description=containedseq[j]->describe();
    const AttributeDescription* attribute;
    description->value >>= attribute;   // Any still owns memory
    result->attributes[j] = *attribute; // deep copy
  }

  result->base_interfaces =baseInterfacesIds();
  result->type            =type();

  return result._retn();
}


AttributeDef_ptr InterfaceDef_impl::create_attribute(
  const char*   id,
  const char*   name,
  const char*   version,
  IDLType_ptr   type,
  AttributeMode mode
)
{
  DB(5,"InterfaceDef::create_attribute("<<id<<","<<name<<","<<version<<",...)")
  checkReadonly();
  checkInheritedNameClash(name);

  // Create the attribute.
  Creche<AttributeDef_impl> newattribute(new AttributeDef_impl());
  newattribute->init(id,name,version,this);
  newattribute->mode(mode);
  newattribute->type_def(type);
  return newattribute.release()->_this();
}


OperationDef_ptr InterfaceDef_impl::create_operation(
  const char*              id,
  const char*              name,
  const char*              version,
  IDLType_ptr              result,
  OperationMode            mode,
  const ParDescriptionSeq& params,
  const ExceptionDefSeq&   exceptions,
  const ContextIdSeq&      contexts
)
{
  DB(5,"InterfaceDef::create_operation("<<id<<","<<name<<","<<version<<",...)")
  checkReadonly();
  checkInheritedNameClash(name);

  // Create the operation.
  Creche<OperationDef_impl> newoperation(new OperationDef_impl());
  newoperation->init(id,name,version,this);
  newoperation->mode(mode);
  newoperation->contexts(contexts);
  newoperation->result_def(result);
  newoperation->params(params);
  newoperation->exceptions(exceptions);
  return newoperation.release()->_this();
}


ContainedSeq* InterfaceDef_impl::contents(
  DefinitionKind limit_type,
  Boolean        exclude_inherited
)
{
  ContainedSeq_var result =Container_impl::contents(limit_type,1);

  // Add in inherited attributes & operations, if required.
  if( (!exclude_inherited) &&
        (limit_type==dk_all ||
         limit_type==dk_Operation ||
         limit_type==dk_Attribute))
  {
    const InterfaceDefSeq& bis=_base_interfaces.in();
    for(ULong i=0; i<bis.length(); ++i)
    {
      // result += inherited operations.
      if( limit_type==dk_all || limit_type==dk_Operation )
      {
        ContainedSeq_var opers =bis[i]->contents(dk_Operation,0);
        ULong offset=result->length();
        result->length( offset+opers->length() );
        for(ULong j=0; j<opers->length(); ++j)
            result[offset+j]=opers[j]._retn();
      }
      // result += inherited attributes.
      if( limit_type==dk_all || limit_type==dk_Operation )
      {
        ContainedSeq_var attrs =bis[i]->contents(dk_Attribute,0);
        ULong offset=result->length();
        result->length( offset+attrs->length() );
        for(ULong k=0; k<attrs->length(); ++k)
            result[offset+k]=attrs[k]._retn();
      }
    }
  }
  
  return result._retn();
}


CORBA::Contained::Description* InterfaceDef_impl::describe()
{
  InterfaceDescription_var interfacedesc = new InterfaceDescription();
  interfacedesc->name           =name();
  interfacedesc->id             =id();
  interfacedesc->version        =version();
  interfacedesc->base_interfaces=baseInterfacesIds();
  interfacedesc->defined_in     =definedInId();

  CORBA::Contained::Description_var description =
      new CORBA::Contained::Description();
  description->kind  =   def_kind();
  description->value <<= interfacedesc._retn();

  return description._retn();
}


TypeCode_ptr InterfaceDef_impl::type()
{
  return Repository_impl::inst()._orb
         ->create_interface_tc(_id.in(),_name.in());
}


void InterfaceDef_impl::uncheckedDestroy()
{
  _base_interfaces.clear();
  Contained_impl::uncheckedDestroy(); // superclass
  Container_impl::uncheckedDestroy(); // superclass
}


bool InterfaceDef_impl::canContain(DefinitionKind kind)
{
  switch(kind)
  {
    CASE_TYPEDEF
    case dk_Constant:
    case dk_Exception:
    case dk_Operation:
    case dk_Attribute:
      return true;
    default:
      return false;
  }
}


void InterfaceDef_impl::reincarnate(const PersistNode& node)
{
  // Reincarnate ths object.
  cdrMemoryStream memstr =node.attrCdrStream("state");
  InterfaceDefSeq b;
  b<<=memstr;
  _base_interfaces.assign(b);
  
  // Reincarnate contained objects.
  // Compare the implementation of Container_impl::recreate()
  // Note that there is no need to defer complete reincarnation of these
  // contained object, because they can never be the object of a link from
  // elsewhere in the hierarchy. (Operations and Attributes can never be
  // referred to by another part of the IDL.)
  for(map<string,PersistNode*>::const_iterator i =node._child.begin();
                                               i!=node._child.end();
					     ++i)
  {
    const string iOid     =i->second->attrString("oid");
    const string iId      =i->second->attrString("id");
    const string iVersion =i->second->attrString("version");
    const string iClass   =i->second->attrString("class");
    const int    iIndex   =i->second->attrLong("index");
    if(iOid.empty() || iId.empty() || iVersion.empty() || iClass.empty())
    {
      DB(1,"InterfaceDef_impl::reincarnate(): "
           "expected 'Contained' type child node.")
      i->second->output(cerr,i->first);
      continue;
    }
    // OK
    if("AttributeDef"==iClass)
    {
      Creche<AttributeDef_impl> servant(new AttributeDef_impl());
      servant->init(iId.c_str(),i->first.c_str(),iVersion.c_str(),this,iIndex);
      servant->reincarnate(*i->second);
      servant.release(iOid.c_str());
    }
    else if("OperationDef"==iClass)
    {
      Creche<OperationDef_impl> servant(new OperationDef_impl());
      servant->init(iId.c_str(),i->first.c_str(),iVersion.c_str(),this,iIndex);
      servant->reincarnate(*i->second);
      servant.release(iOid.c_str());
    }
  } // end for(i)
} // end InterfaceDef_impl::reincarnate()


void InterfaceDef_impl::output(ostream &os)
{
  outputSelf(os,"InterfaceDef");
  cdrMemoryStream memstr(CORBA::ULong(0),CORBA::Boolean(1)/*clearMemory*/);
  _base_interfaces.in()>>=memstr;
  PersistNode::outputCdrMemoryStream(os,memstr,"\n state=");
  os<<" ;;\n";
  outputContents(os);
}


RepositoryIdSeq InterfaceDef_impl::baseInterfacesIds()
{
  RepositoryIdSeq result;
  try
  {
    const InterfaceDefSeq& bis =_base_interfaces.in();
    result.length(bis.length());
    for(ULong i=0; i<bis.length(); ++i)
    {
      assert(!CORBA::is_nil(bis[i]));
      result[i]=bis[i]->id();
    }
  }
  catch(...)
  {
    result.length(0);
    throw;
  }
  return result;
}


void InterfaceDef_impl::checkInheritedNameClash(const char* name)
{
  IdentifierUtil::checkInvalid(name);

  NoCaseStrSet nameSet =inheritedNameSet(_base_interfaces.in());

  if(nameSet.find(string(name))!=nameSet.end())
  {
    DB(1,"BAD_PARAM_InheritedNameClash. InterfaceDef \""<<_name.in()<< \
         "\" rejected duplicate name: \""<<name<<"\"")
    throw CORBA::BAD_PARAM(
      IFELSE_OMNIORB4(omni::BAD_PARAM_InheritedNameClash,5),
      CORBA::COMPLETED_NO
    );
  }
}


void InterfaceDef_impl::interfaceSet(
  const InterfaceDefSeq& baseInterfaces,
  set<string>& repositoryIdSet,
  InterfaceDefSeq& allBaseInterfaces
)
{
  for(ULong i=0; i<baseInterfaces.length(); ++i)
  {
    String_var iName =baseInterfaces[i]->name();
    if(repositoryIdSet.find(string(iName.in()))==repositoryIdSet.end())
    {
      // This approach is inefficient when the sequence length might be large,
      // but for this case, it's OK.
      ULong len =repositoryIdSet.size();
      allBaseInterfaces.length(1+len);
      allBaseInterfaces[len]=CORBA::InterfaceDef::_duplicate(baseInterfaces[i]);
      repositoryIdSet.insert(string(iName.in()));
      // Now add in more distant ancestors.
      InterfaceDefSeq_var ancestors =baseInterfaces[i]->base_interfaces();
      interfaceSet(ancestors.in(),repositoryIdSet,allBaseInterfaces);
    }
  }
}


inline InterfaceDef_impl::NoCaseStrSet
InterfaceDef_impl::inheritedNameSet(
  const InterfaceDefSeq& baseInterfaces
)
{
  // Find all base interfaces, including indirectly inherited ones.
  set<string> ridSet;
  InterfaceDefSeq allBaseInterfaces;
  interfaceSet(baseInterfaces,ridSet,allBaseInterfaces);

  NoCaseStrSet result;
  for(ULong i=0; i<allBaseInterfaces.length(); ++i)
  {
    // ?? Catch COMMS / TRANSIENT errors here? Translate into BAD_PARAM?
    bool ok =true;
    ContainedSeq_var iAttrs =allBaseInterfaces[i]->contents(dk_Attribute,1);
    ContainedSeq_var iOpers =allBaseInterfaces[i]->contents(dk_Operation,1);
    ok=ok&&( nameSetOk(iAttrs.in(),result) );
    ok=ok&&( nameSetOk(iOpers.in(),result) );
    if(!ok) // Conflict amongst base interfaces
    {
      string baseIntNames ="";
      for(set<string>::const_iterator j=ridSet.begin(); j!=ridSet.end(); ++j)
      {
        baseIntNames+=" \""+(*j)+"\"";
      }
      DB(1,"BAD_PARAM_InheritedNameClash. InterfaceDef \""<<_name.in()<< \
        "\" detected conflict amongst base interfaces:"<<baseIntNames.c_str()<<".")
      throw CORBA::BAD_PARAM(
        IFELSE_OMNIORB4(omni::BAD_PARAM_InheritedNameClash,5),
        CORBA::COMPLETED_NO
      );
    }
  }
  return result;
}


inline bool InterfaceDef_impl::nameSetOk(
  const ContainedSeq& contents,
  NoCaseStrSet&       result
)
{
  bool ok =true;
  for(ULong i=0; i<contents.length(); ++i)
  {
    CORBA::String_var iName =contents[i]->name();
    if(!result.insert(iName.in()).second)
    {
      DB(5,"Name clash: \""<<iName.in()<<"\"")
      ok=false; // Name clash detected.
    }
  }
  return ok;
}


} // end namespace Omniifr
