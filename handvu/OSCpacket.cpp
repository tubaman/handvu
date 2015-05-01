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
  * $Id: OSCpacket.cpp,v 1.2 2005/01/18 23:57:42 matz Exp $
**/

// OSCpacket.cpp: constructs OSC packets
//

#include "Common.h"
#include "OSCpacket.h"

#ifdef WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>
#endif

#ifdef USE_MFC
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif // _DEBUG
#endif // USE_MFC


/////////////////////////////////////////////////////////////////////////////
// OSCpacket

OSCpacket::OSCpacket()
  : m_packet_ready(false)
{
  ASSERT(sizeof(char)==1);
  ASSERT(sizeof(int)==4);
  ASSERT(sizeof(float)==4);
}

void OSCpacket::Clear()
{
  m_address = "";
  m_typetags.clear();
  m_values.clear();
  m_packet_ready = false;
}

void OSCpacket::SetAddress(const string& address)
{
  m_address = address;
  CheckAddress();
  m_packet_ready = false;
}

void OSCpacket::AddInt(int val)
{
  char buf[4];
  *((unsigned int*) buf) = htonl(val);
  m_values.push_back(buf[0]);
  m_values.push_back(buf[1]);
  m_values.push_back(buf[2]);
  m_values.push_back(buf[3]);
  m_typetags.push_back('i');
  m_packet_ready = false;
}

void OSCpacket::AddFloat(float val)
{
  char buf[4];
  *((unsigned int*) buf) = htonl(*((int*)&val));
  m_values.push_back(buf[0]);
  m_values.push_back(buf[1]);
  m_values.push_back(buf[2]);
  m_values.push_back(buf[3]);
  m_typetags.push_back('f');
  m_packet_ready = false;
}

void PadBytes(Bytes& bytes) 
{
  int gap = (int)bytes.size()%4;
  if (gap) {
    for (int i=4-gap; i!=0; i--) {
      bytes.push_back(0);
    }
  }
}

void OSCpacket::AddString(const string& val)
{
  for (int i=0; i<(int)val.size(); i++) {
    m_values.push_back(val[i]);
  }
  m_values.push_back('\0');
  PadBytes(m_values);
  m_typetags.push_back('s');
  m_packet_ready = false;
}

const char* OSCpacket::GetBytes() const
{
  if (!m_packet_ready) {
    ((OSCpacket*) this)->CreatePacket();
  }
  return &m_packet[0];
}

int OSCpacket::GetSize() const
{
  if (!m_packet_ready) {
    ((OSCpacket*) this)->CreatePacket();
  }
  return (int) m_packet.size();
}

void OSCpacket::CreatePacket()
{
  ASSERT(!m_packet_ready);
  m_packet.clear();

  // add address and pad
  CheckAddress();
  m_packet.insert(m_packet.end(), m_address.begin(), m_address.end());
  PadBytes(m_packet);

  // add typetags and pad
  m_packet.push_back(',');  // typetag separator
  m_packet.insert(m_packet.end(), m_typetags.begin(), m_typetags.end());
  PadBytes(m_packet);

  // add values and pad
  m_packet.insert(m_packet.end(), m_values.begin(), m_values.end());
  PadBytes(m_packet);

  m_packet_ready = true;
}

void OSCpacket::CheckAddress()
{
  if (m_address.size()<1) throw HVException("OSC address must not be empty");
  if (m_address[0]!='/') throw HVException("OSC address must start with '/'");
  for (int i=0; i<(int)m_address.size(); i++) {
    if (m_address[i]==',') {
      throw HVException("OSC address can not contain ',' for now");
    }
  }
}

