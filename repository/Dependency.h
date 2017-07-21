//                            Package   : omniIFR
//  Dependency.h              Created   : 2004/02/22
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
#ifndef OMNIIFR__DEPENDENCY_H
#define OMNIIFR__DEPENDENCY_H

#include "Repository.h"
#include "IdentifierUtil.h"
#include <string.h> // strcasecmp

namespace Omniifr {

/** Helper method. Converts an object reference into a IRObject_impl pointer.
 * If the conversion succeeds, then the result's servant ref count has been
 * incremented (_add_ref() has been called). It is the caller's responsibility
 * to call _remove_ref() once they are finished with the servant.
 * Deals with all CORBA exceptions that may arise.
 */
inline IRObject_impl* referenceToServant(Object_ptr obj)
{
  IRObject_impl*          result  =NULL;
  PortableServer::Servant servant =NULL;

  try
  {
    servant=Repository_impl::inst()._poa->reference_to_servant(obj);
    // servant's reference count has been incremented for us.
    result=dynamic_cast<IRObject_impl*>(servant);
  }
  catch(PortableServer::POA::ObjectNotActive& ex)
  {
    // _poa does not contain irobject.
    servant=NULL;
    // ?? Temporary diagnostic - help to debug reincarnation:
    // ?? remove this when reincarnation works.
    DB(1,"?? Reference to object that doesn't exist yet. ??")
  }
  catch(PortableServer::POA::WrongAdapter& ex)
  {
    cerr<<"POA has wrong adapter for reference_to_servant()."<<endl;
    exit(1); // Programming error - so quit.
  }
  catch(PortableServer::POA::WrongPolicy& ex)
  {
    cerr<<"POA has wrong policy for reference_to_servant()."<<endl;
    exit(1); // Programming error - so quit.
  }
  catch(Exception& ex)
  {
    DB(1,"Failed to get reference_to_servant"
      IFELSE_OMNIORB4(" ("<<ex._name()<<")",)) // but press on...
  }

  if(servant && !result)
     servant->_remove_ref();// Balances the implicit _add_ref in ref'_to_servant()
  return result;
}

//
//
//

/** Dependency for single references to IRObject.
 * If the _irobject is local, then this method converts it into a local
 * pointer, otherwise it returns NULL. _target may not be nil, but it may
 * refer either to a CORBA object in this InterfaceRepository, or in a
 * remote repository. 
 */
template<class T_IRObject>
class Dependency1 : public DependencyBase
{
public:
  Dependency1(IRObject_impl* owner)
    :DependencyBase(owner),_irobject(T_IRObject::_nil()),_servant(NULL){}
  virtual ~Dependency1(){clear();}
  inline typename T_IRObject::_ptr_type in() {return _irobject.in();}
  inline typename T_IRObject::_ptr_type copy()
  {
    return T_IRObject::_duplicate(_irobject.in());
  }
  inline void assign(typename T_IRObject::_ptr_type irobject)
  {
    typename T_IRObject::_var_type t(irobject); // ensure that irobject ref.
    if(CORBA::is_nil(t.in()))                   //   is released.
        throw CORBA::BAD_PARAM(
          IFELSE_OMNIORB4(omni::BAD_PARAM_InvalidObjectRef,43),
          CORBA::COMPLETED_NO
        );
    clear();
    set(t._retn());
  }
  /** release the current dependency. */
  inline void clear()
  {
    if(_servant)
    {
      _servant->undepend(this);
      _servant=NULL;
    }
    _irobject=T_IRObject::_nil();
  }
private:
  typename T_IRObject::_var_type _irobject;
  IRObject_impl*                 _servant;

  /** establish a new dependency.
   * @param irobject A NON-NIL CORBA reference.
   */
  inline void set(typename T_IRObject::_ptr_type irobject)
  {
    _irobject=irobject;
    _servant=referenceToServant(irobject); // implicit _servant->_add_ref()...
    if(_servant)
    {
      _servant->depend(this);
      _servant->_remove_ref();             // ...cancelled here
    }
  }
}; // end class


////////////////////////////////////////////////////////////////////////////////
// Dependency2
////////////////////////////////////////////////////////////////////////////////

/** Dependency for sequence<IRObject>.
 */
template<class T_IRObjectSeq>
class Dependency2 : public DependencyBase
{
private:
  T_IRObjectSeq        _irobjectSeq;
  list<IRObject_impl*> _servants;
public:
  Dependency2(IRObject_impl* owner)
    :DependencyBase(owner),_irobjectSeq(),_servants(){}
  virtual ~Dependency2(){}
  inline const T_IRObjectSeq& in(){return _irobjectSeq;}
  inline T_IRObjectSeq* copy() {return new T_IRObjectSeq(_irobjectSeq);}
  inline void assign(const T_IRObjectSeq& irobjectSeq)
  {
    check(irobjectSeq); // Throws BAD_PARAM if a problem is found.
    clear();            // Does not throw any exceptions.
    set(irobjectSeq);
  }
  /** release the current dependencies. */
  inline void clear()
  {
    for(typename list<IRObject_impl*>::iterator i=_servants.begin();
        i!=_servants.end();
        ++i)
    {
      if(*i)
        (*i)->undepend(this);
    }
    _servants.clear();
    _irobjectSeq.length(0);
  }
private:

  /** Throws BAD_PARAM if any description in the sequence has the same name
   * as any other, or if a NIL reference is found.
   *
   * @param irobjectSeq sequence<XxxDescription> where XxxDescription is
   *          a sequence of struct that have string member 'name' & IDLType
   *          member 'type_def'.
   */
  inline void check(const T_IRObjectSeq& irobjectSeq)
  {
    for(CORBA::ULong i(0); i<irobjectSeq.length(); i++)
    {
      if(CORBA::is_nil(irobjectSeq[i]))
          throw CORBA::BAD_PARAM(
            IFELSE_OMNIORB4(omni::BAD_PARAM_InvalidObjectRef,43),
            CORBA::COMPLETED_NO
          );
    }
  }
  
  /** establish a new set of dependencies.
   * @param irobjectSeq A NON-NIL CORBA reference.
   */
  inline void set(const T_IRObjectSeq& irobjectSeq)
  {
    assert(_servants.empty());
    assert(0==_irobjectSeq.length());
    _irobjectSeq=irobjectSeq;
    for(CORBA::ULong i(0); i<irobjectSeq.length(); i++)
    {
      IRObject_impl* irobject =referenceToServant(irobjectSeq[i]); // _add_ref()
      if(irobject)
      {
        irobject->depend(this);
        irobject->_remove_ref();                                   // _rem_ref()
      }
      _servants.push_back(irobject);
    }
  }
}; // Dependency2



////////////////////////////////////////////////////////////////////////////////
// Dependency3
////////////////////////////////////////////////////////////////////////////////
/** Dependency for sequence<xxxDescription>, where xxxDescription is a struct
 * with the following members:
 *  - Identifier name
 *  - IRObject   type_def
 */
template<class T_xxxDescriptionSeq>
class Dependency3 : public DependencyBase
{
private:
  T_xxxDescriptionSeq  _descriptionSeq;
  list<IRObject_impl*> _servants;
public:
  Dependency3(IRObject_impl* owner)
    :DependencyBase(owner),_descriptionSeq(),_servants(){}
  virtual ~Dependency3(){}
  inline const T_xxxDescriptionSeq& in(){return _descriptionSeq;}
  inline T_xxxDescriptionSeq* copy()
  {
    return new T_xxxDescriptionSeq(_descriptionSeq);
  }
  inline void assign(const T_xxxDescriptionSeq& descriptionSeq)
  {
    check(descriptionSeq); // Throws BAD_PARAM if a problem is found.
    uncheckedAssign(descriptionSeq);
  }
  /** Performs an assignment but does not call check(). Used when the owner
   * has already permormed its own checks (in UnionDef_impl).
   */
  inline void uncheckedAssign(const T_xxxDescriptionSeq& descriptionSeq)
  {
    clear(); // Does not throw any exceptions.
    set(descriptionSeq);
  }
  /** release the current dependencies. */
  inline void clear()
  {
    for(typename list<IRObject_impl*>::iterator i=_servants.begin();
        i!=_servants.end();
        ++i)
    {
      if(*i)
        (*i)->undepend(this);
    }
    _servants.clear();
    _descriptionSeq.length(0);
  }
private:

  /** Throws BAD_PARAM if any description in the sequence has the same name
   * as any other, or if a NIL reference is found.
   *
   * @param descriptionSeq sequence<XxxDescription> where XxxDescription is
   *          a sequence of struct that have string member 'name' & IDLType
   *          member 'type_def'.
   */
  inline void check(const T_xxxDescriptionSeq& descriptionSeq)
  {
    for(CORBA::ULong i(0); i<descriptionSeq.length(); i++)
    {
      IdentifierUtil::checkInvalid(descriptionSeq[i].name);
      if(CORBA::is_nil(descriptionSeq[i].type_def))
          throw CORBA::BAD_PARAM(
            IFELSE_OMNIORB4(omni::BAD_PARAM_InvalidObjectRef,43),
            CORBA::COMPLETED_NO
          );
      for(CORBA::ULong j(i+1); j<descriptionSeq.length(); j++)
      {
        if(0==strcasecmp(descriptionSeq[i].name,descriptionSeq[j].name))
            throw CORBA::BAD_PARAM(
              IFELSE_OMNIORB4(omni::BAD_PARAM_InvalidMemberName,17),
              CORBA::COMPLETED_NO
            );
      }
    }
  }
  
  /** establish a new set of dependencies.
   * @param descriptionSeq A NON-NIL CORBA reference.
   */
  inline void set(const T_xxxDescriptionSeq& descriptionSeq)
  {
    assert(_servants.empty());
    assert(0==_descriptionSeq.length());
    _descriptionSeq=descriptionSeq;
    for(CORBA::ULong i(0); i<descriptionSeq.length(); i++)
    {
      // referenceToServant() implicitly calls _add_ref()
      IRObject_impl* irobject =referenceToServant(descriptionSeq[i].type_def);
      if(irobject)
      {
        irobject->depend(this);
	// cancel the implicit _add_ref() in referenceToServant().
        irobject->_remove_ref();
      }
      _servants.push_back(irobject);
    }
  }
}; // Dependency3

} // end namespace Omniifr

#endif // OMNIIFR__DEPENDENCY_H
