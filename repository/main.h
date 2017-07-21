//                            Package   : omniIFR
//  main.h                    Created   : 2004/08/01
//                            Author    : Alex Tingle.
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

#ifndef OMNIIFR__MAIN_H
#define OMNIIFR__MAIN_H

/** The main process entry point. Also serves as the 'ServiceMain' entry point
 * on Windows.
 */
int main(int argc, char** argv);

extern "C"
{
  /** Signal handler, sets Orb::_shutdownRequested. The parameter is ignored.
   * This method may be used as a signal handler.
   */
  void Omniifr_Orb_shutdown(int signum);

  /** Signal handler, each call to this method 'bumps' up the trace level by 5,
   * modulo 45.
   */
  void Omniifr_Orb_bumpTraceLevel(int signum);

  /** Signal handler, forces a checkpoint (saves IFR state to file). */
  void Omniifr_Ifr_checkpoint(int signum);
}

#endif // OMNIIFR__MAIN_H
