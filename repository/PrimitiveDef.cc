//                            Package   : omniIFR
//  PrimitiveDef.cc           Created   : 2004/02/22
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
#include "PrimitiveDef.h"
#include <assert.h>

using namespace CORBA;

namespace Omniifr {

void PrimitiveDef_impl::destroy()
{
  // Specification dictates the following exception:
  throw CORBA::BAD_INV_ORDER(
    IFELSE_OMNIORB4(omni::BAD_INV_ORDER_ObjectIndestructible,2),
    CORBA::COMPLETED_NO
  );
}

TypeCode_ptr PrimitiveDef_impl::type()
{
  // cannot be pk_null
  switch(_kind)
  {
    case pk_null:       return TypeCode::_nil();
    case pk_void:       return TypeCode::_duplicate( _tc_void );
    case pk_short:      return TypeCode::_duplicate( _tc_short );
    case pk_long:       return TypeCode::_duplicate( _tc_long );
    case pk_ushort:     return TypeCode::_duplicate( _tc_ushort );
    case pk_ulong:      return TypeCode::_duplicate( _tc_ulong );
    case pk_float:      return TypeCode::_duplicate( _tc_float );
    case pk_double:     return TypeCode::_duplicate( _tc_double );
    case pk_boolean:    return TypeCode::_duplicate( _tc_boolean );
    case pk_char:       return TypeCode::_duplicate( _tc_char );
    case pk_octet:      return TypeCode::_duplicate( _tc_octet );
    case pk_any:        return TypeCode::_duplicate( _tc_any );
    case pk_TypeCode:   return TypeCode::_duplicate( _tc_TypeCode );
    case pk_Principal:  return TypeCode::_duplicate( _tc_Principal );
    case pk_string:     return TypeCode::_duplicate( _tc_string );
    case pk_objref:     return TypeCode::_duplicate( _tc_Object );
#ifdef HAS_LongLong
    case pk_longlong:   return TypeCode::_duplicate( _tc_longlong );
    case pk_ulonglong:  return TypeCode::_duplicate( _tc_ulonglong );
#endif
#ifdef HAS_LongDouble
    case pk_longdouble: return TypeCode::_duplicate( _tc_longdouble );
#endif
    case pk_wchar:      return TypeCode::_duplicate( _tc_wchar );
    case pk_wstring:    return TypeCode::_duplicate( _tc_wstring );
    case pk_value_base: throw NO_IMPLEMENT(); //?? return TypeCode::_duplicate( ?? );

    // Omit default in order that GCC's -Wall will tell us when cases are
    // missing.
  }
  assert(0);
  return TypeCode::_nil();
}


PrimitiveDef_impl::PrimitiveDef_impl(CORBA::PrimitiveKind kind)
: _kind(kind)
{
  switch(_kind)
  {
    case pk_null:       throw BAD_PARAM();
    case pk_void:       activateObjectWithId("pk_void"      ); break;
    case pk_short:      activateObjectWithId("pk_short"     ); break;
    case pk_long:       activateObjectWithId("pk_long"      ); break;
    case pk_ushort:     activateObjectWithId("pk_ushort"    ); break;
    case pk_ulong:      activateObjectWithId("pk_ulong"     ); break;
    case pk_float:      activateObjectWithId("pk_float"     ); break;
    case pk_double:     activateObjectWithId("pk_double"    ); break;
    case pk_boolean:    activateObjectWithId("pk_boolean"   ); break;
    case pk_char:       activateObjectWithId("pk_char"      ); break;
    case pk_octet:      activateObjectWithId("pk_octet"     ); break;
    case pk_any:        activateObjectWithId("pk_any"       ); break;
    case pk_TypeCode:   activateObjectWithId("pk_TypeCode"  ); break;
    case pk_Principal:  activateObjectWithId("pk_Principal" ); break;
    case pk_string:     activateObjectWithId("pk_string"    ); break;
    case pk_objref:     activateObjectWithId("pk_objref"    ); break;
#ifdef HAS_LongLong
    case pk_longlong:   activateObjectWithId("pk_longlong"  ); break;
    case pk_ulonglong:  activateObjectWithId("pk_ulonglong" ); break;
#endif
#ifdef HAS_LongDouble
    case pk_longdouble: activateObjectWithId("pk_longdouble"); break;
#endif
    case pk_wchar:      activateObjectWithId("pk_wchar"     ); break;
    case pk_wstring:    activateObjectWithId("pk_wstring"   ); break;
    case pk_value_base: throw NO_IMPLEMENT();

    // Omit default in order that GCC's -Wall will tell us when cases are
    // missing.
  }
}

} // end namespace Omniifr
