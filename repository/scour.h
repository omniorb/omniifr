/*                            Package   : omniIFR
 * scour.h                    Created   : 2003/10/31
 *                            Author    : Alex Tingle
 *
 *    Copyright (C) 2003 Alex Tingle.
 *
 *    This file is part of the omniIFR application.
 *
 *    omniIFR is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License as published by the Free Software Foundation; either
 *    version 2.1 of the License, or (at your option) any later version.
 *
 *    omniIFR is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* The PACKAGE_* macros cause incompatabilities with omniORB4. Do not export
 * them unless we specifically need the package info.
 * These #undefs cannot be put into config.h.in because configure would
 * then treat them as templates to be overwritten.
 */

#ifndef NEED_PACKAGE_INFO
#  undef PACKAGE_BUGREPORT
#  undef PACKAGE_NAME
#  undef PACKAGE_STRING
#  undef PACKAGE_TARNAME
#  undef PACKAGE_VERSION
#endif
