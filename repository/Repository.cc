//                            Package   : omniIFR
//  Repository.cc             Created   : 2004/02/22
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
#include "Repository.h"

#include "Contained.h"
#include "TypedefDef.h" // for CASE_TYPEDEF macro

#include "PrimitiveDef.h"
#include "StringDef.h"
#include "WstringDef.h"
#include "SequenceDef.h"
#include "ArrayDef.h"
#include "FixedDef.h"
#include "Persist.h"
#include "PersistNode.h"

#include "Creche.h"
#include <string.h>

namespace Omniifr {

/** Helper method used by get_canonical_typecode(). Extracts a narrowed
 * IRObject from the repository, or returns a nil reference. Side effect:
 * sets id & name from the tc.
 */
template<class T>
typename T::_ptr_type lookupTc(
  Repository_impl& repository,
  TypeCode_ptr     tc,
  const char*&     id,  // OUT
  const char*&     name // OUT
)
{
  // Note the non-standard memory management of these functions.
  // id() & name() both return "const char*". The TC retains ownership of the
  // strings.
  id   =tc->id();
  name =tc->name();

  // Attempt to find 'id' in the repository.
  Contained_var contained =repository.lookup_id(id);
  typename T::_var_type result =T::_nil();
  if(!CORBA::is_nil(contained))
  {
    result=T::_narrow(contained.in());
    if(CORBA::is_nil(result))
    {
      DB(1,id<<" is in the repository, but TCKind does not match.") //??
    }
  }
  return result._retn();
}


//
// Repository_impl
//


Repository_impl Repository_impl::_inst; // Default constructed.


Contained_ptr Repository_impl::lookup_id(const char* search_id)
{
  DB(7,"lookup_id(\""<<search_id<<"\")")
  if(!( 0==strcmp(search_id,"IDL:omg.org/CORBA/Object:1.0") ||
        0==strcmp(search_id,"IDL:omg.org/CORBA/ValueBase:1.0")  ))
  {
    Contained_impl* result =findId(search_id);
    if(result)
        return result->_this();
  }
  return Contained::_nil();
}


TypeCode_ptr Repository_impl::get_canonical_typecode(TypeCode_ptr tc)
{
  TypeCode_var result =TypeCode::_nil();

  const char* id;
  const char* name;

  switch(tc->kind())
  {
    // TCs with Repository ID:
    case tk_objref:
      {
        InterfaceDef_var interfaceDef=lookupTc<InterfaceDef>(*this,tc,id,name);
        if(CORBA::is_nil(interfaceDef))
            result=TypeCode::_duplicate(tc); // nothing to add.
        else
            result=interfaceDef->type();
      }
      break;

    case tk_struct:
      {
        StructDef_var structDef=lookupTc<StructDef>(*this,tc,id,name);
        if(CORBA::is_nil(structDef))
        {
          StructMemberSeq ms;
          ms.length(tc->member_count());
          for(ULong i=0; i<ms.length(); ++i)
          {
            TypeCode_var memberType=tc->member_type(i);
            ms[i].name    =CORBA::string_dup(tc->member_name(i)); // [*]
            ms[i].type    =get_canonical_typecode(memberType.in()); // recursive
            ms[i].type_def=IDLType::_nil();
          }
          result=_orb->create_struct_tc(id,name,ms);
        }
        else
        {
          result=structDef->type();
        }
      }
      break;

    case tk_union:
      {
        UnionDef_var unionDef=lookupTc<UnionDef>(*this,tc,id,name);
        if(CORBA::is_nil(unionDef))
        {
          UnionMemberSeq ms;
          ms.length(tc->member_count());
          for(ULong i=0; i<ms.length(); ++i)
          {
            TypeCode_var memberType  =tc->member_type(i);
            Any_var      memberLabel =tc->member_label(i);
            ms[i].name    =CORBA::string_dup(tc->member_name(i)); // [*]
            ms[i].label   =memberLabel.in();
            ms[i].type    =get_canonical_typecode(memberType.in()); // recursive
            ms[i].type_def=IDLType::_nil();
          }
          TypeCode_var      discType =tc->discriminator_type();
          TypeCode_var canonDiscType =get_canonical_typecode(discType.in());
          result=_orb->create_union_tc(id,name,canonDiscType.in(),ms);
        }
        else
        {
          result=unionDef->type();
        }
      }
      break;

    case tk_enum:
      {
        EnumDef_var enumDef=lookupTc<EnumDef>(*this,tc,id,name);
        if(CORBA::is_nil(enumDef))
            result=TypeCode::_duplicate(tc); // nothing to add.
        else
            result=enumDef->type();
      }
      break;

    case tk_alias:
      {
        AliasDef_var aliasDef=lookupTc<AliasDef>(*this,tc,id,name);
        if(CORBA::is_nil(aliasDef))
        {
          TypeCode_var      contType =tc->content_type();
          TypeCode_var canonContType =get_canonical_typecode(contType.in());
          result=_orb->create_alias_tc(id,name,canonContType.in());
        }
        else
        {
          result=aliasDef->type();
        }
      }
      break;

    case tk_except:
      {
        ExceptionDef_var exceptionDef=lookupTc<ExceptionDef>(*this,tc,id,name);
        if(CORBA::is_nil(exceptionDef))
        {
          StructMemberSeq ms;
          ms.length(tc->member_count());
          for(ULong i=0; i<ms.length(); ++i)
          {
            TypeCode_var memberType=tc->member_type(i);
            ms[i].name    =CORBA::string_dup(tc->member_name(i)); // [*]
            ms[i].type    =get_canonical_typecode(memberType.in()); // recursive
            ms[i].type_def=IDLType::_nil();
          }
          result=_orb->create_exception_tc(id,name,ms);
        }
        else
        {
          result=exceptionDef->type();
        }
      }
      break;

    case tk_sequence:
      {
        TypeCode_var      contType =tc->content_type();
        TypeCode_var canonContType =get_canonical_typecode(contType.in());
        result=_orb->create_sequence_tc(tc->length(),canonContType.in());
      }
      break;

    case tk_array:
      {
        TypeCode_var      contType =tc->content_type();
        TypeCode_var canonContType =get_canonical_typecode(contType.in());
        result=_orb->create_array_tc(tc->length(),canonContType.in());
      }
      break;

    // Primitives
    case tk_null:
    case tk_void:
    case tk_short:
    case tk_long:
    case tk_ushort:
    case tk_ulong:
    case tk_float:
    case tk_double:
    case tk_boolean:
    case tk_char:
    case tk_octet:
    case tk_any:
    case tk_TypeCode:
    case tk_Principal:
    case tk_string:
#ifdef HAS_LongLong
    case tk_longlong:
    case tk_ulonglong:
#endif
#ifdef HAS_LongDouble
    case tk_longdouble:
#endif
    case tk_wchar:
    case tk_wstring:
    case tk_fixed:
      {
        result=TypeCode::_duplicate(tc);
      }
      break;

    // WTF? Not implemented in omniORB?
    case tk_value:
    case tk_value_box:
    case tk_native:
    case tk_abstract_interface:
    case tk_local_interface:
      {
        cerr<<"Arrgh! Help!"<<endl; //??
        result=TypeCode::_duplicate(tc);
      }
      break;
      
    case _np_tk_indirect: // Internal to omniORB. We should never get this.
      assert(0);

  } // end case. Note: no default, so that missing options are flagged by GCC.

  assert(!CORBA::is_nil(result));
  return result._retn();
} // end Repository_impl::get_canonical_typecode


PrimitiveDef_ptr Repository_impl::get_primitive(PrimitiveKind kind)
{
  if(kind==pk_null)
      return PrimitiveDef::_nil();

  map<PrimitiveKind,PrimitiveDef_impl*>::iterator pos=_primitives.find(kind);
  if(pos==_primitives.end())
      throw NO_IMPLEMENT();

  assert(pos->second);
  return pos->second->_this();
}


StringDef_ptr Repository_impl::create_string(ULong bound)
{
  DB(5,"Repository::create_string("<<bound<<")")
  checkReadonly();
  Creche<StringDef_impl> result(new StringDef_impl(bound));
  return result.release()->_this();
}


WstringDef_ptr Repository_impl::create_wstring(ULong bound) 
{
  DB(5,"Repository::create_wstring("<<bound<<")")
  checkReadonly();
  Creche<WstringDef_impl> result(new WstringDef_impl(bound));
  return result.release()->_this();
}


SequenceDef_ptr Repository_impl::create_sequence(
  ULong       bound,
  IDLType_ptr element_type
)
{
  DB(5,"Repository::create_sequence("<<bound<<",...)")
  checkReadonly();
  Creche<SequenceDef_impl> result(new SequenceDef_impl(bound));
  result->element_type_def(element_type);
  return result.release()->_this();
}


ArrayDef_ptr Repository_impl::create_array(
  ULong       length,
  IDLType_ptr element_type
)
{
  DB(5,"Repository::create_array("<<length<<",...)")
  checkReadonly();
  Creche<ArrayDef_impl> result(new ArrayDef_impl(length));
  result->element_type_def(element_type);
  return result.release()->_this();
}


FixedDef_ptr Repository_impl::create_fixed(UShort digits, Short scale)
{
  DB(5,"Repository::create_fixed("<<digits<<","<<scale<<")")
  checkReadonly();
  Creche<FixedDef_impl> result(new FixedDef_impl(digits,scale));
  return result.release()->_this();
}


void Repository_impl::init(
  CORBA::ORB_ptr orb,
  bool           readonly,
  Persist*       persist
)
{
  _orb=orb; // Store a reference to orb.
  _readonly=readonly;

  const char* action=""; // Use this variable to help report errors.
  try
  { 
    action="resolve initial reference 'omniINSPOA'";
    CORBA::Object_var obj =orb->resolve_initial_references("omniINSPOA");
    _omniINSPOA=PortableServer::POA::_narrow(obj);
    if(CORBA::is_nil(_omniINSPOA))
        throw CORBA::OBJECT_NOT_EXIST(0,CORBA::COMPLETED_NO);

    action="resolve initial reference 'RootPOA'";
    obj=orb->resolve_initial_references("RootPOA");
    PortableServer::POA_var rootPoa =PortableServer::POA::_narrow(obj);
    if(CORBA::is_nil(rootPoa))
        throw CORBA::OBJECT_NOT_EXIST(0,CORBA::COMPLETED_NO);
    
    action="create Interface Repository's POA";
    createPoa(rootPoa.in());

    action="resolve initial reference 'DynAnyFactory'";
    obj=_orb->resolve_initial_references("DynAnyFactory");
    _DynAnyFactory=DynamicAny::DynAnyFactory::_narrow(obj);
    if(CORBA::is_nil(_DynAnyFactory))
        throw CORBA::OBJECT_NOT_EXIST(0,CORBA::COMPLETED_NO);

    action="create primitives."; // (They auto-activate)
    _primitives[pk_void]      =new PrimitiveDef_impl(pk_void);
    _primitives[pk_short]     =new PrimitiveDef_impl(pk_short);
    _primitives[pk_long]      =new PrimitiveDef_impl(pk_long);
    _primitives[pk_ushort]    =new PrimitiveDef_impl(pk_ushort);
    _primitives[pk_ulong]     =new PrimitiveDef_impl(pk_ulong);
    _primitives[pk_float]     =new PrimitiveDef_impl(pk_float);
    _primitives[pk_double]    =new PrimitiveDef_impl(pk_double);
    _primitives[pk_boolean]   =new PrimitiveDef_impl(pk_boolean);
    _primitives[pk_char]      =new PrimitiveDef_impl(pk_char);
    _primitives[pk_octet]     =new PrimitiveDef_impl(pk_octet);
    _primitives[pk_any]       =new PrimitiveDef_impl(pk_any);
    _primitives[pk_TypeCode]  =new PrimitiveDef_impl(pk_TypeCode);
    _primitives[pk_Principal] =new PrimitiveDef_impl(pk_Principal);
    _primitives[pk_string]    =new PrimitiveDef_impl(pk_string);
    _primitives[pk_objref]    =new PrimitiveDef_impl(pk_objref);
#ifdef HAS_LongLong
    _primitives[pk_longlong]  =new PrimitiveDef_impl(pk_longlong);
    _primitives[pk_ulonglong] =new PrimitiveDef_impl(pk_ulonglong);
#endif
#ifdef HAS_LongDouble
    _primitives[pk_longdouble]=new PrimitiveDef_impl(pk_longdouble);
#endif
    _primitives[pk_wchar]     =new PrimitiveDef_impl(pk_wchar);
    _primitives[pk_wstring]   =new PrimitiveDef_impl(pk_wstring);

    action="activate the Repository's POA";
    PortableServer::POAManager_var pman =_poa->the_POAManager();
    pman->activate();

    action="activate the INS POA";
    pman =_omniINSPOA->the_POAManager();
    pman->activate();
    
    action="start up persistency";
    if(persist)
       persist->startup();

    action="activate InterfaceRepository 'DefaultRepository'";
    activateObjectWithId("DefaultRepository");
  }
  catch(CORBA::ORB::InvalidName& ex) // resolve_initial_references
  {
    cerr<<"Failed to "<<action<<". InvalidName"<<endl;
    throw;
  }
  catch(CORBA::TRANSIENT& ex) // _narrow()
  {
    cerr<<"Failed to "<<action<<". TRANSIENT"<<endl;
    throw;
  }
  catch(CORBA::OBJECT_NOT_EXIST& ex) // _narrow()
  {
    cerr<<"Failed to "<<action<<". OBJECT_NOT_EXIST"<<endl;
    throw;
  }
  catch(CORBA::SystemException& ex)
  {
    cerr<<"Failed to "<<action<<"."
      IFELSE_OMNIORB4(" "<<ex._name()<<" ("<<ex.NP_minorString()<<")",0) <<endl;
    throw;
  }
  catch(CORBA::Exception& ex)
  {
    cerr<<"Failed to "<<action<<"." IFELSE_OMNIORB4(" "<<ex._name(),0) <<endl;
    throw;
  }
}


Repository_impl::Repository_impl():
 _primitives(),
 _anonymous(),
 _idmap(),
 _readonly(false)
{}


Repository_impl::~Repository_impl()
{}


void Repository_impl::uncheckedDestroy()
{
  for(map<PrimitiveKind,PrimitiveDef_impl*>::iterator i=_primitives.begin();
      i!=_primitives.end();
      ++i)
  {
    assert(i->second);
    i->second->uncheckedDestroy();
  }
  _primitives.clear();
}


bool Repository_impl::canContain(DefinitionKind kind)
{
  switch(kind)
  {
    CASE_TYPEDEF
    case dk_Constant:
    case dk_Exception:
    case dk_Interface:
    case dk_Value:
    case dk_ValueBox:
    case dk_Module:
      return true;
    default:
      return false;
  }
}


void Repository_impl::addId(string id, Contained_impl* container)
{
  assert(!findId(id));
  _idmap[id]=container;
}


void Repository_impl::removeId(string id)
{
  map<string,Contained_impl*>::iterator pos =_idmap.find(id);
  assert(pos!=_idmap.end());
  _idmap.erase(pos);
}


Contained_impl* Repository_impl::findId(string id)
{
  map<string,Contained_impl*>::iterator pos =_idmap.find(id);
  if(pos==_idmap.end())
      return NULL;
  else
      return pos->second;
}


void Repository_impl::addAnonymous(IDLType_impl* anon)
{
  // Compare the implementation of this method to that of
  // Container::addContained().
  assert(anon);
  this->_add_ref(); // Do this now to ensure we aren't destroyed during the call
  try
  {
    // Verify that we don't already contain it.
    assert(_anonymous.find(anon)==_anonymous.end());
    // Check that we are able to contain the specified type.
    CORBA::DefinitionKind anonKind =anon->def_kind();
    switch(anonKind)
    {
      case dk_String:
      case dk_Wstring:
      case dk_Sequence:
      case dk_Array:
      case dk_Fixed:
        break;

      default:
        throw CORBA::BAD_PARAM(
          IFELSE_OMNIORB4(omni::BAD_PARAM_TargetIsInvalidContainer,4),
          CORBA::COMPLETED_NO
        );
    }
    // proceed...
    _anonymous.insert(anon);
  }
  catch(...)
  {
    this->_remove_ref();
    throw;
  }
}
        
void Repository_impl::removeAnonymous(IDLType_impl* anon)
{
  set<IDLType_impl*>::iterator pos =_anonymous.find(anon);
  assert(pos!=_anonymous.end());
  _anonymous.erase(pos);
  this->_remove_ref(); // Removes the ref added during ::addAnonymous()
}

void Repository_impl::createPoa(PortableServer::POA_ptr rootPoa)
{
  using namespace PortableServer;
  try
  {
    // POLICIES:
    //  Lifespan          =PERSISTENT             // we can persist
    //  Assignment        =USER_ID                // write our own oid
    //  Uniqueness        =[default] UNIQUE_ID    // one servant per object
    //  ImplicitActivation=[default] IMPLICIT_ACTIVATION // auto activation
    //  RequestProcessing =[default] USE_ACTIVE_OBJECT_MAP_ONLY
    //  ServantRetention  =[default] RETAIN       // stateless POA
    //  Thread            =SINGLE_THREAD_MODEL    // keep it simple

    CORBA::PolicyList policies;
    policies.length(3);
    policies[0]=rootPoa->create_lifespan_policy(PERSISTENT);
    policies[1]=rootPoa->create_id_assignment_policy(USER_ID);
    policies[2]=rootPoa->create_thread_policy(SINGLE_THREAD_MODEL);

    // Create a new POA (and new POAManager) for this Repository.
    _poa=rootPoa->create_POA("IRpoa",POAManager::_nil(),policies);
  }
  catch(POA::AdapterAlreadyExists& ex) // create_POA
  {
    cerr<<"POA::AdapterAlreadyExists"<<endl;
    throw;
  }
  catch(POA::InvalidPolicy& ex) // create_POA
  {
    cerr<<"POA::InvalidPolicy: "<<ex.index<<endl;
    throw;
  }
}


void Repository_impl::reincarnate(const PersistNode &node)
{
  // Reincarnation is a two-stage process:
  // Firstly we must recreate all of the CORBA objects.
  // Secondly we recreate the links between those objects.

  map<IRObject_impl*,PersistNode*> todo;

  // Recreate anonymous types.
  PersistNode* idlTypeNode;
  map<string,PersistNode*>::const_iterator i;
  if(idlTypeNode=node.child("StringDef"))
  {
    for(i=idlTypeNode->_child.begin(); i!=idlTypeNode->_child.end(); ++i)
    {
      CORBA::ULong bound =i->second->attrLong("bound");
      DB(5,"Repository::reincarnate string("<<bound<<")")
      Creche<StringDef_impl> servant(new StringDef_impl(bound));
      servant.release( i->first.c_str() ); // Ignore servant
    }
  }
  if(idlTypeNode=node.child("WstringDef"))
  {
    for(i=idlTypeNode->_child.begin(); i!=idlTypeNode->_child.end(); ++i)
    {
      CORBA::ULong bound =i->second->attrLong("bound");
      DB(5,"Repository::reincarnate wstring("<<bound<<")")
      Creche<WstringDef_impl> servant(new WstringDef_impl(bound));
      servant.release( i->first.c_str() ); // Ignore servant
    }
  }
  if(idlTypeNode=node.child("SequenceDef"))
  {
    for(i=idlTypeNode->_child.begin(); i!=idlTypeNode->_child.end(); ++i)
    {
      CORBA::ULong bound =i->second->attrLong("bound");
      DB(5,"Repository::reincarnate sequence("<<bound<<")")
      Creche<SequenceDef_impl> servant(new SequenceDef_impl(bound));
      todo[servant.get()]=i->second;
      servant.release( i->first.c_str() ); // Ignore servant
    }
  }
  if(idlTypeNode=node.child("ArrayDef"))
  {
    for(i=idlTypeNode->_child.begin(); i!=idlTypeNode->_child.end(); ++i)
    {
      CORBA::ULong length =i->second->attrLong("length");
      DB(5,"Repository::reincarnate array("<<length<<")")
      Creche<ArrayDef_impl> servant(new ArrayDef_impl(length));
      todo[servant.get()]=i->second;
      servant.release( i->first.c_str() ); // Ignore servant
    }
  }
  if(idlTypeNode=node.child("FixedDef"))
  {
    for(i=idlTypeNode->_child.begin(); i!=idlTypeNode->_child.end(); ++i)
    {
      CORBA::UShort digits =i->second->attrLong("digits");
      CORBA::Short  scale  =i->second->attrLong("scale");
      DB(5,"Repository::reincarnate fixed("<<digits<<","<<scale<<")")
      Creche<FixedDef_impl> servant(new FixedDef_impl(digits,scale));
      servant.release( i->first.c_str() ); // Ignore servant
    }
  }
  
  // Recreate contained objects.
  recreate(node.child("NAME"),todo/*OUT*/);

  // Create links.
  for(map<IRObject_impl*,PersistNode*>::iterator i2 =todo.begin();
                                                 i2!=todo.end();
                                               ++i2)
  {
    DB(5,"Repository::reincarnate "<<i2->second->attrString("id").c_str())
    i2->first->reincarnate(*i2->second);
  }
}


void Repository_impl::output(ostream &os)
{
  // Output anonymous types.
  for(set<IDLType_impl*>::iterator i=_anonymous.begin();
      i!=_anonymous.end();
      ++i)
  {
    (**i).output(os);
  }
  // Output named types.
  outputContents(os);
}

PortableServer::POA_ptr Repository_impl::_default_POA()
{
  return PortableServer::POA::_duplicate(_omniINSPOA.in());
}

} // end namespace Omniifr
