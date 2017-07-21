//                            Package   : omniIFR
// Persist.cc                 Created   : 2004/09/05
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

#include "Persist.h"

#include "Repository.h"
#include "PersistNode.h"
#include <memory>

namespace Omniifr {


Persist::Persist(int checkpointPeriodSec)
: _ifr(Omniifr::Repository_impl::inst()),
  _lock(),
  _outstream(NULL),
  _checkpointNeeded(true),
  _dir("/var/lib/omniifr")
{}


Persist::~Persist()
{
  if(_outstream)
  {
    _outstream->close();
    delete _outstream;
  }
}


void Persist::startup()
{
  assert(_outstream==NULL);
  {
    string fname =_dir+"/omniifr.db";
    ifstream instream(fname.c_str());
    if(instream)
    {
      auto_ptr<PersistNode> savedState( new PersistNode(instream) );
      instream.close();
      _ifr.reincarnate(*savedState);
    }
  }
  
  // ?? START UP CHECKPOINTING HERE
}


void Persist::checkpoint()
{
  // ?? This is a dummy implementation.
  // The real implementation should move aside the current database file
  // before replacing it with a new one. See the very robust (if a bit cruddy)
  // implementation in omniEvents.
  // ?? Furthermore, this is currently called from a signal handler - it would
  // be safer for that call to be replaced by a condition_variable wakeup.
  _ifr.output(outstream());
  outstream()<<flush;
}


ostream& Persist::outstream()
{
  if(!_outstream)
  {
    string fname =_dir+"/omniifr.db";
    _outstream=new ofstream(fname.c_str());
  }
  return *_outstream;
}


} // end namespace Omniifr
