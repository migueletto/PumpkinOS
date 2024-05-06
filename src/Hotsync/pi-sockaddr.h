/*
 * $Id: pi-sockaddr.h,v 1.10 2006/10/17 13:24:07 desrod Exp $
 *
 * pi-sockaddr.h
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _PILOT_SOCKADDR_H_
#define _PILOT_SOCKADDR_H_

struct pi_sockaddr {
  unsigned short pi_family;
  char 	pi_device[255];
};

#endif /* _PILOT_SOCKADDR_H_ */
