//                            Package   : omniIFR
//  string_to.h               Created   : 2005/02/20
//                            Author    : Alex Tingle
//
//    Copyright (C) 2005 Alex Tingle.
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
#ifndef OMNIIFR__STRING_TO_H
#define OMNIIFR__STRING_TO_H

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifdef HAVE_OMNIORB3
#  include <omniORB3/CORBA.h>
#endif

#ifdef HAVE_OMNIORB4
#  include <omniORB4/CORBA.h>
#endif

#include "Repository.h"

namespace Omniifr {

/** Converts a string to a narrowed reference. */
template<class T>
typename T::_ptr_type string_to_(const char* oidStr)
{
  CORBA::Object_var obj =Repository_impl::inst()._orb->string_to_object(oidStr);

  if(CORBA::is_nil(obj.in()))
      throw CORBA::BAD_PARAM();

#ifdef HAVE_OMNIORB4
  typename T::_var_type result =T::_unchecked_narrow(obj);
#else
  typename T::_var_type result =T::_narrow(obj);
#endif

  if(CORBA::is_nil(result.in()))
      throw CORBA::BAD_PARAM();

  return result._retn();
}

} // end namespace Omniifr

#endif // OMNIIFR__STRING_TO_H
