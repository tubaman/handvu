/**
  * HandVu - a library for computer vision-based hand gesture
  * recognition.
  * Copyright (C) 2004 Mathias Kolsch, matz@cs.ucsb.edu
  *
  * This program is free software; you can redistribute it and/or
  * modify it under the terms of the GNU General Public License
  * as published by the Free Software Foundation; either version 2
  * of the License, or (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program; if not, write to the Free Software
  * Foundation, Inc., 59 Temple Place - Suite 330, 
  * Boston, MA  02111-1307, USA.
  *
  * $Id: OSCpacket.h,v 1.1 2005/01/18 22:02:42 matz Exp $
**/

// OSCpacket.h: header file for OSC packet constructor

#if !defined(__OSCPACKET_H__INCLUDED_)
#define __OSCPACKET_H__INCLUDED_

#include "Exceptions.h"

#include <string>
#include <vector>
using namespace std;

typedef vector<char> Bytes;


class OSCpacket {
 public:
  OSCpacket();

  void Clear();
  void SetAddress(const string& address);
  void AddInt(int val);
  void AddFloat(float val);
  void AddString(const string& val);

  const char* GetBytes() const;
  int GetSize() const;

 protected:
  void CreatePacket();
  void CheckAddress();

 protected:
  bool                           m_packet_ready;
  Bytes                          m_packet;

  string                         m_address;
  Bytes                          m_typetags;
  mutable Bytes                  m_values;
};


#endif // __OSCPACKET_H__INCLUDED_

