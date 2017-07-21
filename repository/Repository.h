//                            Package   : omniIFR
//  Repository.h              Created   : 2004/02/22
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
#ifndef OMNIIFR__REPOSITORY_H
#define OMNIIFR__REPOSITORY_H

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifdef HAVE_IOSTREAM
#  include <iostream>
#else
#  include <iostream.h>
#endif

#include "Container.h"

#include <string>
#include <map>

namespace Omniifr {

class PrimitiveDef_impl;
class IDLType_impl;
class Contained_impl;
class Persist;
class PersistNode;

class Repository_impl :
  public virtual POA_CORBA::Repository,
  public         Container_impl
{
////////////////////////////////////////////////////////////////////////////////
// Singleton stuff.
private:
  static Repository_impl _inst;
public:
  static Repository_impl& inst() {return _inst;}
  void init(ORB_ptr orb, bool readonly=false, Persist* persist=NULL);
  ORB_ptr                       _orb;
  PortableServer::POA_var       _poa;
  PortableServer::POA_var       _omniINSPOA;
  DynamicAny::DynAnyFactory_var _DynAnyFactory;
////////////////////////////////////////////////////////////////////////////////

public: // CORBA interface methods

  // methods that read
  Contained_ptr    lookup_id(const char* search_id); ///< typedef: RepositoryId
  TypeCode_ptr     get_canonical_typecode(TypeCode_ptr tc);
  PrimitiveDef_ptr get_primitive(PrimitiveKind kind);

  // methods that write
  StringDef_ptr   create_string(   ULong  bound );
  WstringDef_ptr  create_wstring(  ULong  bound );
  SequenceDef_ptr create_sequence( ULong  bound,  IDLType_ptr element_type );
  ArrayDef_ptr    create_array(    ULong  length, IDLType_ptr element_type );
  FixedDef_ptr    create_fixed(    UShort digits, Short       scale );

public: // CORBA interface methods (from IRObject)
  DefinitionKind  def_kind(){return dk_Repository;}


private:
  map<PrimitiveKind,PrimitiveDef_impl*> _primitives;
  set<IDLType_impl*>                    _anonymous;
  map<string,Contained_impl*>           _idmap;
  bool                                  _readonly;

public:
  Repository_impl();
  virtual ~Repository_impl();
  void uncheckedDestroy();
  void createPoa(PortableServer::POA_ptr rootPoa);
  bool canContain(DefinitionKind kind);
  void addId(string id, Contained_impl* container);
  void removeId(string id);
  Contained_impl* findId(string id);
  void addAnonymous(IDLType_impl* anon);
  void removeAnonymous(IDLType_impl* anon);
  bool readonly() const { return _readonly; }

  /** This is the top-level reincarnate() method, it calls all of the
   * other ones.
   */
  void reincarnate(const PersistNode& node);
  void output(ostream &os);

protected:
  PortableServer::POA_ptr _default_POA();
}; // Repository_impl

} // end namespace Omniifr

#endif // OMNIIFR__REPOSITORY_H
