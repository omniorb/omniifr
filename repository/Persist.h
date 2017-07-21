//                            Package   : omniIFR
// Persist.h                  Created   : 2004/09/05
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

#ifndef OMNIIFR__PERSIST_H
#define OMNIIFR__PERSIST_H 1

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifdef HAVE_IOSTREAM
#  include <fstream>
#else
#  include <fstream.h>
#endif

#ifdef HAVE_STD_IOSTREAM
using namespace std;
#endif

#include <omnithread.h>

namespace Omniifr {

class Repository_impl;

/** Responsible for reading & writing the persistancy database. */
class Persist
{
public:
  Persist(int checkpointPeriodSec);
  ~Persist();

  /** Sets the directory used for persistency database files. */  
  void setdir(const char* dir) { _dir=dir; }
  
  /**
   * Called by the Repository, when initial setup is complete.
   * Reincarnates the repository contents (if there is a database file from
   * which to read) and [?? FUTURE:] starts the checkpointing thread.
   */
  void startup();
  
  /** [?? FUTURE:] Called by the checkpointing thread, and also when
   * Omniifr_Ifr_checkpoint() receives SIGUSR2. Writes the entire
   * contents of the repository to a clean database file.
   */
  void checkpoint();
  
  /** Obtains an open output stream. */
  ostream& outstream();

  /** Obtains an output stream to the active persistancy file, and locks it for
   * exclusive access. The lock is released when the object is destructed. */
  class WriteLock
  {
  public:
    inline WriteLock(Persist& p);
    inline ~WriteLock();
    ostream& os;
  private:
    Persist&        persist;
    omni_mutex_lock lk;
    WriteLock(const WriteLock&); ///< No implementation
  };
  friend class WriteLock;
  
private:
  Repository_impl& _ifr;
  omni_mutex       _lock;
  ofstream*        _outstream;
  bool             _checkpointNeeded;
  string           _dir;
};


//
// Inline method implementations
//


inline Persist::WriteLock::WriteLock(Persist& p):
  os(p.outstream()),
  persist(p),
  lk(p._lock)
{}

inline Persist::WriteLock::~WriteLock()
{
  os.flush();
  persist._checkpointNeeded=true;
}

} // end namespace Omniifr

#endif // OMNIIFR__PERSIST_H
