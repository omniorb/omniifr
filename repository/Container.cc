//                            Package   : omniIFR
//  Container.cc              Created   : 2004/02/22
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
#include "Container.h"

#include "Contained.h"
#include "Repository.h"
#include "ModuleDef.h"

#include "ConstantDef.h"
#include "StructDef.h"
#include "UnionDef.h"
#include "EnumDef.h"
#include "AliasDef.h"
#include "InterfaceDef.h"
#include "ExceptionDef.h"

#include "PersistNode.h"
#include "Creche.h"

#include <string.h>
#include <algorithm>

namespace Omniifr {

Contained_ptr Container_impl::lookup(const char* search_name)
{
  Contained_impl* result =lookupServant(search_name,true); // Case sensitive
  if(result)
      return result->_this();
  else
      return Contained::_nil();
}


/** This default implementation does not implement inheritance, see subclasses
 * for that.
 */
ContainedSeq* Container_impl::contents(
  DefinitionKind limit_type,
  CORBA::Boolean exclude_inherited
)
{
  ContainedSeq_var result = new ContainedSeq();
  ULong resultLength =0;

  for(list<Contained_impl*>::iterator it =_contents.begin();
      it!=_contents.end();
      ++it)
  {
    if(limit_type==dk_all || limit_type==(*it)->def_kind())
    {
      result->length(resultLength+1);
      result[resultLength]=(*it)->_this();
      ++resultLength;
    }
  }
  return result._retn();
}

/** There is no special code to implement inheritance here. That is delegated
 * to the polymorphic contents() operation.
 */
ContainedSeq* Container_impl::lookup_name(
  const char*    search_name,
  CORBA::Long    levels_to_search,
  DefinitionKind limit_type,
  CORBA::Boolean exclude_inherited
)
{
  ContainedSeq_var result =new ContainedSeq();
  ULong resultLength =0;
  Long subLevels =levels_to_search<0? levels_to_search : levels_to_search-1;
  EqualName equalName(search_name,true); // true -> case sensitive

  // Obtain all contents (even inherited) that match the limit_type.
  ContainedSeq_var contents =this->contents(limit_type,exclude_inherited);
  for(ULong i=0; i<contents->length(); ++i)
  {
    if(equalName(contents[i]) && // name match
        limit_type==dk_all || limit_type==contents[i]->def_kind()) // type match
    {
      result->length(resultLength+1);
      result[resultLength]=contents[i];
      ++resultLength;
    }
  }

  // Search sub levels, if required.
  if(subLevels!=0)
  {
    for(list<Contained_impl*>::iterator it =_contents.begin();
        it!=_contents.end();
        ++it)
    {
      Container_impl* subContainer =dynamic_cast<Container_impl*>(*it);
      if(subContainer)
      {
        ContainedSeq_var subResult =subContainer->
          lookup_name(search_name,subLevels,limit_type,exclude_inherited);
        result->length( resultLength+subResult->length() );
        for(ULong j=0; j<subResult->length(); ++j)
            result[resultLength++]=subResult[j]._retn();
      }
    }
  }
  return result._retn();
}

CORBA::Container::DescriptionSeq* Container_impl::describe_contents(
  DefinitionKind limit_type,
  Boolean        exclude_inherited,
  Long           max_returned_objs
)
{
  ContainedSeq_var containedseq =contents(limit_type,exclude_inherited);

  ULong numObjectsToReturn =containedseq->length();
  if(max_returned_objs>=Long(numObjectsToReturn))
      numObjectsToReturn=max_returned_objs;     

  CORBA::Container::DescriptionSeq_var result =
    new CORBA::Container::DescriptionSeq();
  result->length(numObjectsToReturn);

  for(ULong i=0; i<numObjectsToReturn; ++i)
  {
    CORBA::Contained::Description_var iDescription =containedseq[i]->describe();
    result[i].contained_object=containedseq[i];
    result[i].kind            =iDescription->kind;
    result[i].value           =iDescription->value;
  }
  return result._retn();
}

ModuleDef_ptr Container_impl::create_module(
  const char* id, const char* name, const char* version
)
{
  DB(5,"Container::create_module("<<id<<","<<name<<","<<version<<")")
  checkReadonly();
  Creche<ModuleDef_impl> newmodule(new ModuleDef_impl());
  newmodule->init(id,name,version,this);
  return newmodule.release()->_this();
}

ConstantDef_ptr Container_impl::create_constant(
  const char* id, const char* name, const char* version,
  IDLType_ptr type,
  const Any&  value
)
{
  DB(5,"Container::create_constant("<<id<<","<<name<<","<<version<<",...)")
  checkReadonly();
  Creche<ConstantDef_impl> newconstant(new ConstantDef_impl());
  newconstant->init(id,name,version,this);
  // Order is important.
  newconstant->type_def(type);
  newconstant->value(value);
  return newconstant.release()->_this();
}

StructDef_ptr Container_impl::create_struct(
  const char* id, const char* name, const char* version,
  const StructMemberSeq& members
)
{
  DB(5,"Container::create_struct("<<id<<","<<name<<","<<version<<",...)")
  checkReadonly();
  Creche<StructDef_impl> newstruct(new StructDef_impl());
  newstruct->init(id,name,version,this);
  newstruct->members(members);
  return newstruct.release()->_this();
}

UnionDef_ptr Container_impl::create_union(
  const char* id, const char* name, const char* version,
  IDLType_ptr           discriminator_type,
  const UnionMemberSeq& members
) 
{
  DB(5,"Container::create_union("<<id<<","<<name<<","<<version<<",...)")
  checkReadonly();
  Creche<UnionDef_impl> newunion(new UnionDef_impl());
  newunion->init(id,name,version,this);
  // Note, order is important.
  newunion->discriminator_type_def(discriminator_type);
  newunion->members(members);
  return newunion.release()->_this();
}

EnumDef_ptr Container_impl::create_enum(
  const char* id, const char* name, const char* version,
  const EnumMemberSeq& members
)
{
  DB(5,"Container::create_enum("<<id<<","<<name<<","<<version<<",...)")
  checkReadonly();
  Creche<EnumDef_impl> newenum(new EnumDef_impl());
  newenum->init(id,name,version,this);
  newenum->members(members);
  return newenum.release()->_this();
}

AliasDef_ptr Container_impl::create_alias(
  const char* id, const char* name, const char* version,
  IDLType_ptr original_type
)
{
  DB(5,"Container::create_alias("<<id<<","<<name<<","<<version<<",...)")
  checkReadonly();
  Creche<AliasDef_impl> newalias(new AliasDef_impl());
  newalias->init(id,name,version,this);
  newalias->original_type_def(original_type);
  return newalias.release()->_this();
}

InterfaceDef_ptr Container_impl::create_interface(
  const char* id, const char* name, const char* version,
  const InterfaceDefSeq& base_interfaces
)
{
  DB(5,"Container::create_interface("<<id<<","<<name<<","<<version<<",...)")
  checkReadonly();
  Creche<InterfaceDef_impl> newinterface(new InterfaceDef_impl());
  newinterface->init(id,name,version,this);
  newinterface->base_interfaces(base_interfaces);
  return newinterface.release()->_this();
}

ExceptionDef_ptr Container_impl::create_exception(
  const char* id, const char* name, const char* version,
  const StructMemberSeq& members
)
{
  DB(5,"Container::create_exception("<<id<<","<<name<<","<<version<<",...)")
  checkReadonly();
  Creche<ExceptionDef_impl> newexception(new ExceptionDef_impl());
  newexception->init(id,name,version,this);
  newexception->members(members);
  return newexception.release()->_this();
}

ValueDef_ptr Container_impl::create_value(
  const char* id, const char* name, const char* version,
  Boolean                is_custom,
  Boolean                is_abstract,
  ValueDef_ptr           base_value,
  Boolean                is_truncatable,
  const ValueDefSeq&     abstract_base_values,
  const InterfaceDefSeq& supported_interfaces,
  const InitializerSeq&  initializers
)
{
  DB(5,"Container::create_value("<<id<<","<<name<<","<<version<<",...)")
  checkReadonly();
  // not supported
  return ValueDef::_nil();
}

ValueBoxDef_ptr Container_impl::create_value_box(
  const char* id, const char* name, const char* version,
  IDLType_ptr original_type_def
)
{
  DB(5,"Container::create_value_box("<<id<<","<<name<<","<<version<<",...)")
  checkReadonly();
  // not supported
  return ValueBoxDef::_nil();    
}

NativeDef_ptr Container_impl::create_native(
  const char* id, const char* name, const char* version
)
{
  DB(5,"Container::create_native("<<id<<","<<name<<","<<version<<",...)")
  checkReadonly();
  // not supported
  return NativeDef::_nil();
}

AbstractInterfaceDef_ptr Container_impl:: create_abstract_interface(
  const char* id, const char* name, const char* version,
  const AbstractInterfaceDefSeq& base_interfaces
)
{
  DB(5,"Container::create_abstract_interface("<<id<<","<<name<<","<<version<<",...)")
  checkReadonly();
  // not supported
  return AbstractInterfaceDef::_nil();
}

//
// added members
//

void Container_impl::dependentObjectSet(set<const IRObject_impl*>& result) const
{
  IRObject_impl::dependentObjectSet(result); // parent's implementation.
  // now add in contained objects
  for(list<Contained_impl*>::const_iterator i=_contents.begin();
      i!=_contents.end();
      ++i)
  {
    (**i).dependentObjectSet(result);
  }
}

void Container_impl::containedObjectSet(set<const IRObject_impl*>& result) const
{
  IRObject_impl::containedObjectSet(result); // parent's implementation.
  // now add in contained objects
  for(list<Contained_impl*>::const_iterator i=_contents.begin();
      i!=_contents.end();
      ++i)
  {
    (**i).containedObjectSet(result);
  }
}

void Container_impl::recreate(
  PersistNode*                      node,
  map<IRObject_impl*,PersistNode*>& todo)
{
  if(!node)
  {
    DB(1,"Container_impl::recreate(): NULL child node encountered.")
    return;
  }

  for(map<string,PersistNode*>::const_iterator i =node->_child.begin();
                                               i!=node->_child.end();
					     ++i)
  {
    const string iOid     =i->second->attrString("oid");
    const string iId      =i->second->attrString("id");
    const string iVersion =i->second->attrString("version");
    const string iClass   =i->second->attrString("class");
    const int    iIndex   =i->second->attrLong("index");
    if(iOid.empty() || iId.empty() || iVersion.empty() || iClass.empty())
    {
      DB(1,"Container_impl::recreate(): expected 'Contained' type child node.")
      i->second->output(cerr,i->first);
      continue;
    }
    // OK
    if("ModuleDef"==iClass)
    {
      Creche<ModuleDef_impl> servant(new ModuleDef_impl());
      servant->init(iId.c_str(),i->first.c_str(),iVersion.c_str(),this,iIndex);
      servant->recreate(i->second,todo);
      servant.release(iOid.c_str());
    }
    else if("ConstantDef"==iClass)
    {
      Creche<ConstantDef_impl> servant(new ConstantDef_impl());
      servant->init(iId.c_str(),i->first.c_str(),iVersion.c_str(),this,iIndex);
      todo[servant.get()]=i->second;
      servant.release(iOid.c_str());
    }
    else if("StructDef"==iClass)
    {
      Creche<StructDef_impl> servant(new StructDef_impl());
      servant->init(iId.c_str(),i->first.c_str(),iVersion.c_str(),this,iIndex);
      servant->recreate(i->second,todo);
      todo[servant.get()]=i->second;
      servant.release(iOid.c_str());
    }
    else if("UnionDef"==iClass)
    {
      Creche<UnionDef_impl> servant(new UnionDef_impl());
      servant->init(iId.c_str(),i->first.c_str(),iVersion.c_str(),this,iIndex);
      servant->recreate(i->second,todo);
      todo[servant.get()]=i->second;
      servant.release(iOid.c_str());
    }
    else if("EnumDef"==iClass)
    {
      Creche<EnumDef_impl> servant(new EnumDef_impl());
      servant->init(iId.c_str(),i->first.c_str(),iVersion.c_str(),this,iIndex);
      todo[servant.get()]=i->second;
      servant.release(iOid.c_str());
    }
    else if("AliasDef"==iClass)
    {
      Creche<AliasDef_impl> servant(new AliasDef_impl());
      servant->init(iId.c_str(),i->first.c_str(),iVersion.c_str(),this,iIndex);
      todo[servant.get()]=i->second;
      servant.release(iOid.c_str());
    }
    else if("InterfaceDef"==iClass)
    {
      Creche<InterfaceDef_impl> servant(new InterfaceDef_impl());
      servant->init(iId.c_str(),i->first.c_str(),iVersion.c_str(),this,iIndex);
      servant->recreate(i->second,todo);
      todo[servant.get()]=i->second;
      servant.release(iOid.c_str());
    }
    else if("ExceptionDef"==iClass)
    {
      Creche<ExceptionDef_impl> servant(new ExceptionDef_impl());
      servant->init(iId.c_str(),i->first.c_str(),iVersion.c_str(),this,iIndex);
      servant->recreate(i->second,todo);
      todo[servant.get()]=i->second;
      servant.release(iOid.c_str());
    }
    else if("AttributeDef"!=iClass && "OperationDef"!=iClass)
    {
      DB(1,"Container_impl::recreate(): "
           "unexpected child class "<<iClass.c_str())
    }
  } // end for(i)
} // end Container_impl::recreate()

void Container_impl::outputContents(ostream &os) const
{
  for(list<Contained_impl*>::const_iterator i=_contents.begin();
      i!=_contents.end();
      ++i)
  {
    (**i).output(os);
  }
}

Container_impl::Container_impl() : _contents()
{
  _contents.clear();
}

void Container_impl::uncheckedDestroy() 
{
  for(list<Contained_impl*>::const_iterator i=_contents.begin();
      i!=_contents.end();
      ++i)
  {
    (**i).uncheckedDestroy();
  }
  _contents.clear();
}

Contained_impl* Container_impl::lookupServant(
  const char* searchName,
  bool        matchCase
)
{
  assert(searchName);
  if(!searchName[0])// Empty string
      return NULL;

  // Split into: first::theRest
  const char* offset =strstr(searchName,"::");
  string first(searchName,offset?offset-searchName:strlen(searchName));
  string theRest(offset?offset+2:"");

  Contained_impl* result =NULL;

  if(first.empty())
  {
    result=Repository_impl::inst().lookupServant(theRest.c_str(),matchCase);
  }
  else
  {
    list<Contained_impl*>::iterator pos =
      find_if(
        _contents.begin(),
        _contents.end(),
        EqualName(first.c_str(),matchCase)
      );
    if(pos!=_contents.end()) // match found
    {
      if(theRest.empty())
      {
        result=*pos;
      }
      else
      {
        Container_impl* container =dynamic_cast<Container_impl*>(*pos);
        if(container)
            result=container->lookupServant(theRest.c_str(),matchCase);
      }
    }
  }
  return result;
}

void Container_impl::addContained(Contained_impl* contained)
{
  // Precondition: We know that the contained->name is not already in use by
  // this container, either because we've checked for that in
  // Contained_impl::name() or because we've set it to "" during a
  // Contained_impl::move().

  this->_add_ref(); // Do this now to ensure we aren't destroyed during the call
  try
  {
    // Verify that we don't already contain it.
    assert(find_if(_contents.begin(),_contents.end(),
                   bind2nd(equal_to<Contained_impl*>(),contained))
           ==_contents.end()
    );
    // Check that we are able to contain the specified type.
    if(!canContain(contained->def_kind()))
        throw CORBA::BAD_PARAM(
          IFELSE_OMNIORB4(omni::BAD_PARAM_TargetIsInvalidContainer,4),
          CORBA::COMPLETED_NO
        );
    // proceed...
    if(contained->_index > 0)
    {
      // we are being reincarnated: Put the it back in the right place.
      list<Contained_impl*>::iterator pos=_contents.begin();
      while(pos!=_contents.end() && (*pos)->_index < contained->_index)
          ++pos;
      _contents.insert(pos,contained);
    }
    else
    {
      // A newly added object - put it at the end.
      contained->_index=(_contents.empty()? 1: 1+_contents.back()->_index);
      _contents.push_back(contained);
    }
  }
  catch(...)
  {
    this->_remove_ref();
    throw;
  }
}

void Container_impl::removeContained(Contained_impl* contained)
{
  list<Contained_impl*>::iterator pos =
    find_if(_contents.begin(),_contents.end(),
            bind2nd(equal_to<Contained_impl*>(),contained));
  assert(pos!=_contents.end());
  _contents.erase(pos);
  this->_remove_ref();
}

bool Container_impl::EqualName::operator()(Contained_impl* c)
{
  return test(c->name());
}

} // end namespace Omniifr
