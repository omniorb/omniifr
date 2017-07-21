//                            Package   : omniIFR
//  WstringDef.cc             Created   : 2004/02/22
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
#include "WstringDef.h"
#include "Repository.h"
#include "PersistNode.h"

namespace Omniifr {

TypeCode_ptr WstringDef_impl::type()
{
  return Repository_impl::inst()._orb->create_wstring_tc(_bound);
}

WstringDef_impl::WstringDef_impl(CORBA::ULong bound)
:_bound(bound)
{
  Repository_impl::inst().addAnonymous(this);
}

void WstringDef_impl::uncheckedDestroy()
{
  Repository_impl::inst().removeAnonymous(this);
}

void WstringDef_impl::output(ostream &os)
{
  os<<"WstringDef"<<PersistNode::_separator;
  outputOid(os);
  os<<" bound="<<_bound<<" ;;\n";
}

} // end namespace Omniifr
