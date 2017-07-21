//                            Package   : omniIFR
//  IfrConnection.h           Created   : 2004/03/21
//                            Author    : Alex Tingle
//
//    This work is hereby released into the Public Domain. To view a copy of
//    the public domain dedication, visit
//    http://creativecommons.org/licenses/publicdomain/ or send a letter to
//    Creative Commons, 559 Nathan Abbott Way, Stanford, California 94305, USA.
// 

//
// Note: Must be compiled with the macro ENABLE_CLIENT_IR_SUPPORT set.
// E.g. -DENABLE_CLIENT_IR_SUPPORT=1
//

#ifndef OMNIIFR_IFRCONNECTION_H
#define OMNIIFR_IFRCONNECTION_H

#include <omniORB4/CORBA.h>
#include <assert.h>

/** Encapsulates a connection to the Interface Repository. */
class IfrConnection
{
public:
  CORBA::Repository_var ifr;

  /** Constructor, obtains a reference to the Interface Repository from
   * resolve_initial_references().
   */
  IfrConnection(CORBA::ORB_ptr orb)
  {
    using namespace CORBA;
    assert(!is_nil(orb));
    Object_var ifrObj =orb->resolve_initial_references("InterfaceRepository");
    ifr=Repository::_narrow(ifrObj);
  }

  /** Constructor, is passed a reference to the Interface Repository.
   */
  IfrConnection(CORBA::Repository_ptr ifr)
  {
    assert(!CORBA::is_nil(ifr));
    this->ifr=ifr;
  }

  /** Looks up the definition of obj's interface in the InterfaceRepository.
   * Returns a nil reference if the definition is not found in the 
   * repository, or if obj is an UNNARROWED reference.
   *
   * This method uses omniORB internals. It is non portable. It is ONLY intended
   * as a workaround alternative to CORBA::Object::_get_interface(), while
   * that method's implementation is broken.
   */
  CORBA::InterfaceDef_ptr get_interface(CORBA::Object_ptr obj)
  {
    using namespace CORBA;
    assert(!is_nil(ifr));
    omniObjRef* objRef =obj->_PR_getobj();
    if(objRef && (long)objRef!=1) // Set to 1 for pseudo references.
    {
      const char* rid =objRef->_mostDerivedRepoId();
      if(!rid || !rid[0])
      {
        cout<<"_mostDerivedRepoId() empty. Falling back to _localServantTarget()"<<endl;
        rid =objRef->_localServantTarget();
      }
      Contained_var contained =ifr->lookup_id(rid);
      if(!is_nil(contained))
          return InterfaceDef::_narrow(contained.in());
    }
    return InterfaceDef::_nil();
  }

};

#endif // OMNIIFR_IFRCONNECTION_H
