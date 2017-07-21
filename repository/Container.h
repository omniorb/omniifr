//                            Package   : omniIFR
//  Container.h               Created   : 2004/02/22
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
#ifndef OMNIIFR__CONTAINER_H
#define OMNIIFR__CONTAINER_H

#include "IRObject.h"

#include <list>
#include <map>
#include <string.h>

using namespace CORBA;

namespace Omniifr {

class Contained_impl;
class PersistNode;

class Container_impl:
  public virtual POA_CORBA::Container,
  public virtual IRObject_impl
{
public: // CORBA interface methods

  // methods that read

  /** "The lookup operation locates a definition relative to this container
   * given a scoped name using OMG IDL's name scoping rules. An absolute scoped
   * name (beginning with "::") locates the definition relative to the
   * enclosing Repository. If no object is found, a nil object reference is
   * returned." -OMG
   */
  Contained_ptr lookup(
    const char*    search_name ///< typedef: ScopedName
  );

  /**
   * "The contents operation returns the list of objects directly contained by
   * or inherited into the object. The operation is used to navigate through
   * the hierarchy of objects. Starting with the Repository object, a client
   * uses this operation to list all of the objects contained by the
   * Repository, all of the objects contained by the modules within the
   * Repository, and then all of the interfaces and value types within a
   * specific module, and so on." -OMG
   *
   * @param limit_type "If limit_type is set to dk_all "all," objects of all
   *          interface types are returned. For example, if this is an
   *          InterfaceDef, the attribute, operation, and exception objects
   *          are all returned. If limit_type is set to a specific interface,
   *          only objects of that interface type are returned. For example,
   *          only attribute objects are returned if limit_type is set to
   *          dk_Attribute "AttributeDef"." -OMG
   *
   * @param exclude_inherited "If set to TRUE, inherited objects (if there are
   *          any) are not returned. If set to FALSE, all contained
   *          objects whether contained due to inheritance or because they
   *          were defined within the object are returned." -OMG
   */
  virtual ContainedSeq* contents(
    DefinitionKind limit_type,
    Boolean        exclude_inherited
  );

  /**
   * "The lookup_name operation is used to locate an object by name within a
   * particular object or within the objects contained by that object. Use of
   * values of levels_to_search of 0 or of negative numbers other than -1 is
   * undefined." -OMG
   *
   * @param search_name Specifies which name is to be searched for.
   *
   * @param levels_to_search "Controls whether the lookup is constrained to
   *          the object the operation is invoked on or whether it should
   *          search through objects contained by the object as well." -OMG
   *
   *          "Setting levels_to_search to -1 searches the current object and
   *          all contained objects. Setting levels_to_search to 1 searches
   *          only the current object. Use of values of levels_to_search of
   *          0 or of negative numbers other than -1 is undefined." -OMG
   */
  ContainedSeq* lookup_name(
    const char*    search_name, ///< typedef: Identifier
    Long           levels_to_search,
    DefinitionKind limit_type,
    Boolean        exclude_inherited
  );

  /**
   * "The describe_contents operation combines the contents operation and the
   * describe operation. For each object returned by the contents operation, the
   * description of the object is returned (i.e., the object's describe
   * operation is invoked and the results returned)." -OMG
   *
   * @param max_returned_objs "Limits the number of objects that can be returned
   *          in an invocation of the call to the number provided. Setting the
   *          parameter to -1 means return all contained objects." -OMG
   */
  CORBA::Container::DescriptionSeq* describe_contents(
    DefinitionKind limit_type,
    Boolean        exclude_inherited,
    Long           max_returned_objs
  );

  // methods that write
  ModuleDef_ptr create_module(
    const char* id,      ///< typedef: RepositoryId
    const char* name,    ///< typedef: Identifier
    const char* version  ///< typedef: VersionSpec
  );

  ConstantDef_ptr create_constant(
    const char* id,      ///< typedef: RepositoryId
    const char* name,    ///< typedef: Identifier
    const char* version, ///< typedef: VersionSpec
    IDLType_ptr type,
    const Any&  value
  );

  StructDef_ptr create_struct(
    const char* id,      ///< typedef: RepositoryId
    const char* name,    ///< typedef: Identifier
    const char* version, ///< typedef: VersionSpec
    const StructMemberSeq& members
  );

  UnionDef_ptr create_union(
    const char* id,      ///< typedef: RepositoryId
    const char* name,    ///< typedef: Identifier
    const char* version, ///< typedef: VersionSpec
    IDLType_ptr discriminator_type,
    const UnionMemberSeq& members
  );

  EnumDef_ptr create_enum(
    const char* id,      ///< typedef: RepositoryId
    const char* name,    ///< typedef: Identifier
    const char* version, ///< typedef: VersionSpec
    const EnumMemberSeq& members
  );

  AliasDef_ptr create_alias(
    const char* id,      ///< typedef: RepositoryId
    const char* name,    ///< typedef: Identifier
    const char* version, ///< typedef: VersionSpec
    IDLType_ptr original_type
  );

  InterfaceDef_ptr create_interface(
    const char* id,      ///< typedef: RepositoryId
    const char* name,    ///< typedef: Identifier
    const char* version, ///< typedef: VersionSpec
    const InterfaceDefSeq& base_interfaces
  );

  ExceptionDef_ptr create_exception(
    const char* id,      ///< typedef: RepositoryId
    const char* name,    ///< typedef: Identifier
    const char* version, ///< typedef: VersionSpec
    const StructMemberSeq& members
  );

  /** Not supported. */
  ValueDef_ptr create_value(
    const char* id,      ///< typedef: RepositoryId
    const char* name,    ///< typedef: Identifier
    const char* version, ///< typedef: VersionSpec
    Boolean                is_custom,
    Boolean                is_abstract,
    ValueDef_ptr           base_value,
    Boolean                is_truncatable,
    const ValueDefSeq&     abstract_base_values,
    const InterfaceDefSeq& supported_interfaces,
    const InitializerSeq&  initializers
  );

  /** Not supported. */
  ValueBoxDef_ptr create_value_box(
    const char* id,      ///< typedef: RepositoryId
    const char* name,    ///< typedef: Identifier
    const char* version, ///< typedef: VersionSpec
    IDLType_ptr original_type_def
  );

  /** Not supported. */
  NativeDef_ptr create_native(
    const char* id,      ///< typedef: RepositoryId
    const char* name,    ///< typedef: Identifier
    const char* version  ///< typedef: VersionSpec
  );

  /** Not supported (deprecated) */
  AbstractInterfaceDef_ptr create_abstract_interface(
    const char* id,      ///< typedef: RepositoryId
    const char* name,    ///< typedef: Identifier
    const char* version, ///< typedef: VersionSpec
    const AbstractInterfaceDefSeq& base_interfaces
  );

  /* Not implemented in omniORB.
  LocalInterfaceDef_ptr create_local_interface(
    const char* id,      ///< typedef: RepositoryId
    const char* name,    ///< typedef: Identifier
    const char* version, ///< typedef: VersionSpec
    const InterfaceDefSeq& base_interfaces
  );
  */

protected:
  void dependentObjectSet(set<const IRObject_impl*>& result) const;
  void containedObjectSet(set<const IRObject_impl*>& result) const;
  
  /** Recreate contained objects from a PersistNode.
   * Does not recreate links & members - instead the object->node relationship
   * is added to the 'todo' map. This is later used by
   * Repository_impl::reincarnate() to complete the reincarnation process.
   */
  void recreate(PersistNode* node, map<IRObject_impl*,PersistNode*>& todo);
  
  /** Helper method, calls output method for each Contained. */
  void outputContents(ostream &os) const;

public:
  Container_impl();
  virtual ~Container_impl() {}
  virtual void uncheckedDestroy() =0;

  /** Works exactly like lookup(), except the caller can choose whether they
   * want case-sensitive or case insensitive searching. The returned value is
   * a servant pointer rather than a CORBA object reference.
   */
  Contained_impl* lookupServant(
    const char* searchName, ///< typedef: ScopedName
    bool        matchCase
  );

  /** Returns TRUE if this Container_impl can contain type 'kind'.
   * Legal relationships are defined in CORBA spec. section 10.4.4 "Structure
   * and Navigation of the Interface Repository".
   */
  virtual bool canContain(DefinitionKind kind)=0;

  void addContained(Contained_impl* contained);
  void removeContained(Contained_impl* contained);

private:
  list<Contained_impl*> _contents;

  /** Predicate that tests a Contained->name() for equality
   * with the 'name' parameter. Overloaded for local servant pointers and
   * CORBA object references.
   */
  struct EqualName{
    EqualName(const char* name,bool matchCase):_name(name),_case(matchCase){}
    bool operator()(Contained_ptr c){return test(c->name());}
    bool operator()(Contained_impl* c);
    /** Returns (name==_name). Consumes parameter. */
    bool test(char* name) {
      String_var cname(name); // consumes parameter
      if(_case)
        return(0==strcmp(cname.in(),_name.in()));
      else
        return(0==strcasecmp(cname.in(),_name.in()));
    }
    CORBA::String_var _name;
    bool              _case;
  };
}; // Container_impl

} // end namespace Omniifr

#endif // OMNIIFR__CONTAINER_H
