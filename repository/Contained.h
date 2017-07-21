//                            Package   : omniIFR
//  Contained.h               Created   : 2004/02/22
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
#ifndef OMNIIFR__CONTAINED_H
#define OMNIIFR__CONTAINED_H

#include "IRObject.h"

using namespace CORBA;

namespace Omniifr {

class Repository_impl;
class Container_impl;

class Contained_impl :
  public virtual POA_CORBA::Contained,
  public virtual IRObject_impl
{
public: // CORBA interface methods

  // read/write attributes
          char* id();
          void  id( const char* );
          char* name();
          void  name( const char* );
          char* version();
          void  version( const char* );
  // readonly attributes
          Container_ptr  defined_in();
          char*          absolute_name();
          Repository_ptr containing_repository();
  // methods
  virtual CORBA::Contained::Description* describe() =0;
          void move(Container_ptr, const char*, const char*);

public:
  Contained_impl(){}
  /** Initialize the container immediately after construction. This method must
   * be called immediately after construction or member variables will not be
   * set and fundamental assumptions about data validity will be violated.
   *
   * So, why not do this in the constructor? Well, some of the code relies upon
   * virtual methods that cannot be relied upon during base class construction.
   *
   * @param index when non-zero: used to place this object within its Container.
   */
  void init(
    const char*     id,
    const char*     name,
    const char*     version,
    Container_impl* defined_in,
    int             index =0
  );
  virtual ~Contained_impl() {}
  virtual void uncheckedDestroy() =0;
  /** Documents the ordering of Contained objects within their Container.
   * Zero means not yet ordered.
   */
  int _index;
protected:
  RepositoryId_var _id;
  Identifier_var   _name;
  VersionSpec_var  _version;

  /** Helper method. Returns this->defined_in()->id(), or an empty string. */
  char* definedInId();

  /** Helper method. Sends a persistency file header, and common 'Contained'
   * attributes to the output stream:
   *
   *  Format:
   *    NAME<absolute_name> oid=<oid>
   *     id=<id>
   *     version=<version>
   *     class=<className>
   */
  void outputSelf(ostream &os, const char* className)
  {
    os<<"NAME"<<_absolute_name.in()<<" oid=";
    outputOid(os);
    os<<"\n id="<<_id.in()<<
        "\n version="<<_version.in()<<
        "\n class="<<className<<
        "\n index="<<_index;
  }

private:
  Container_impl*  _defined_in;
  ScopedName_var   _absolute_name;
private:
  /** Sets the value of _absolute_name from _name. */
  void updateAbsoluteName();
}; // Contained_impl

} // end namespace Omniifr

#endif // OMNIIFR__CONTAINED_H
