//                            Package   : omniIFR
//  IdentifierUtil.h          Created   : 2004/03/19
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
#ifndef OMNIIFR__IDENTIFIERUTIL_H
#define OMNIIFR__IDENTIFIERUTIL_H

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifdef HAVE_OMNIORB3
#  include <omniORB3/CORBA.h>
#endif

#ifdef HAVE_OMNIORB4
#  include <omniORB4/CORBA.h>
#endif

#include <ctype.h> // tolower
#include <string.h> // strcmp, strcasecmp

namespace Omniifr {

/** Utility class containing useful methods for manipulating IDL identifiers.
 */
class IdentifierUtil
{
public:

  enum Match{
    equalMatch,      ///< Identifiers match exactly.
    equivalentMatch, ///< Identifiers only match case-insensitively.
    noMatch          ///< Identifiers do not match, even case-insensitively.
  };
  
  inline static Match compare(const char* n1, const char* n2)
  {
    if(!n1 || !n2)
        return noMatch;
    else if(0==strcmp(n1,n2))
        return equalMatch;
    else if(0==strcasecmp(n1,n2))
        return equivalentMatch;
    else
        return noMatch;
  }

  /** Generates a BAD_PARAM exception if name is not a valid IDL identifier.
   * Valid identifiers match this Regex: [a-zA-Z][_a-zA-Z0-9]*
   */
  inline static void checkInvalid(const char* name)
  {
    // Should not be testing NULL pointers.
    assert(name);

    bool valid =bool( name[0] ); // string not empty

    if(valid)
    {
      for(int i=0; name[i]; ++i)
      {
        char c =tolower(name[i]);
        if( ('a'<=c && c<='z') ||
            (i>0 &&
              ( ('0'<=c && c<='9') ||
                c=='_'
          ) ) )
        {
          continue; // still valid
        }
        valid=false;
        break;
      }
    }

    if(!valid)
    {
      throw CORBA::BAD_PARAM(
        IFELSE_OMNIORB4(omni::BAD_PARAM_InvalidName,15),
        CORBA::COMPLETED_NO
      );
    }
  }
};

} // end namespace Omniifr

#endif // OMNIIFR__IDENTIFIERUTIL_H
