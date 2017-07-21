//                            Package   : omniIFR
//  Creche.h                  Created   : 2004/02/22
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

#ifndef OMNIIFR__CRECHE_H
#define OMNIIFR__CRECHE_H

namespace Omniifr {

/** Used to hold IRObject_impl objects while they are being constructed.
 * Automatically calls uncheckedDestroy() if the destructer is called without
 * the contents first being removed. Ensures that they are properly cleaned up
 * if an exception is raised.
 */
template<class T>
class Creche
{
public:
  Creche(T* ptr):_ptr(ptr){assert(_ptr);}
  ~Creche()
  {
    if(_ptr)
    {
      _ptr->uncheckedDestroy();
      _ptr->_remove_ref(); // Kill the constructor's reference to this.
      _ptr=NULL;
    }
  }
  T* get()        const {return _ptr;}
  T* operator->() const {return _ptr;}
  /** Activates the object and releases it into the 'wild'.
   * If an ObjectID is provided, then the object is activated with that ID,
   * otherwise a new random ID is assigned.
   */
  T* release(const char* oidStr =NULL)
  {
    assert(_ptr);
    T* result=_ptr;
    if(oidStr)
        _ptr->activateObjectWithId(oidStr);
    else
        _ptr->activateObject();
    _ptr=NULL;
    return result;
  }
private:
  Creche(const Creche&); ///< No implementation
  T*   _ptr;
};

} // end namespace Omniifr

#endif // OMNIIFR__CRECHE_H
