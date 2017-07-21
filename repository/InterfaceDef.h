//                            Package   : omniIFR
//  InterfaceDef.h            Created   : 2004/02/22
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
#ifndef OMNIIFR__INTERFACEDEF_H
#define OMNIIFR__INTERFACEDEF_H

#include "Container.h"
#include "Contained.h"
#include "idltype.h"
#include "Dependency.h"
#include <string>
#include <set>

namespace Omniifr {

class Container_impl; 
class Repository_impl;

class InterfaceDef_impl :
  public virtual POA_CORBA::InterfaceDef,
  public         Container_impl,
  public         Contained_impl,
  public         IDLType_impl
{
public: // CORBA attributes
  InterfaceDefSeq* base_interfaces();
  void             base_interfaces(const InterfaceDefSeq& v);
public: // CORBA interface methods
  Boolean is_a(const char* interface_id);
  CORBA::InterfaceDef::FullInterfaceDescription* describe_interface();
  AttributeDef_ptr create_attribute(
    const char*   id,
    const char*   name,
    const char*   version,
    IDLType_ptr   type,
    AttributeMode mode
  );
  OperationDef_ptr create_operation(
    const char*              id,
    const char*              name,
    const char*              version,
    IDLType_ptr              result,
    OperationMode            mode,
    const ParDescriptionSeq& params,
    const ExceptionDefSeq&   exceptions,
    const ContextIdSeq&      contexts
  );
public: // CORBA interface methods (from IRObject)
  DefinitionKind def_kind(){return CORBA::dk_Interface;}
public: // CORBA interface methods (from Container)
  ContainedSeq* contents(DefinitionKind limit_type, Boolean exclude_inherited);
public: // CORBA interface methods (from Contained)
  CORBA::Contained::Description* describe();
public: // CORBA attributes (from IDLType)
  TypeCode_ptr type();

private:
  Dependency2<InterfaceDefSeq> _base_interfaces;

public:
  InterfaceDef_impl():_base_interfaces(this){}
  virtual ~InterfaceDef_impl() {}
  void uncheckedDestroy();
  bool canContain(DefinitionKind kind);
  void reincarnate(const PersistNode& node);
  void output(ostream &os);

private:
  /** Obtain a RepositoryIdSeq from _base_interfaces. */
  RepositoryIdSeq baseInterfacesIds();

  /** Helper method called by create_operation/attribute(). Checks that 'name'
   * is a valid identifier, and does not clash with an inherited name. Throws
   * BAD_PARAM if a problem is found.
   */
  void checkInheritedNameClash(const char* name);

  /** Comparison operator used by NoCaseStrSet */
  struct NoCaseStringOrdering
  {
    bool operator()(string s1, string s2) const
    {
      return( strcasecmp(s1.c_str(),s2.c_str())<0 );
    }
  };

  typedef set<string,NoCaseStringOrdering> NoCaseStrSet;

  /** Calculate the set of all unique interfaces from which baseInterfaces
   * are derived.
   */
  void interfaceSet(
    const InterfaceDefSeq& baseInterfaces,
    set<string>&           repositoryIdSet,
    InterfaceDefSeq&       allBaseInterfaces
  );

  /** Calculate the set of all attribute & operation names contained by
   * 'baseInterfaces'. Throws BAD_PARAM(InheritedNameClash) if a duplicate is
   * found.
   */
  inline NoCaseStrSet inheritedNameSet(
    const InterfaceDefSeq& baseInterfaces
  );

  /** Sets 'result' to the set union of 'result's initial value and the set
   * of all contents[x]->name().
   * Returns FALSE if a duplicate is found.
   */
  inline static bool nameSetOk(
    const ContainedSeq& contents,
    NoCaseStrSet&        result
  );
};

} // end namespace Omniifr

#endif // OMNIIFR__INTERFACEDEF_H
