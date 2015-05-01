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
  * $Id: GestureServer.h,v 1.5 2005/10/30 21:16:03 matz Exp $
**/

// GestureServer.h: TCP/IP and UDP server
//

#include "HandVu.hpp"  // for HVState only
#include "OSCpacket.h"
#include <string>
#include <list>
using namespace std;

#ifdef WIN32
#include <winsock2.h>
#else
typedef int SOCKET;
#endif


#define GESTUREEVENT_VERSION_STRING "1.2"

typedef list<SOCKET> SocketList;

class GestureServer {

 public:
  GestureServer& operator=(const GestureServer& from)    ;

  virtual void Start() = 0;
  virtual void Stop() = 0;
  virtual void Send(const HVState& state) = 0;

  bool IsStarted() const {return m_started;}

 protected:
  GestureServer();
  void SetNonBlocking(SOCKET sd);

 protected:
  bool                           m_started;
};


class GestureServerStream : public GestureServer {

 public:
  GestureServerStream(int port, int max_num_clients=10);
  GestureServerStream(const GestureServerStream& from);
  ~GestureServerStream();
  GestureServerStream& operator=(const GestureServerStream& from);

  virtual void Start();
  virtual void Stop();
  virtual void Send(const HVState& state);

 protected:
  void InitSockets();
  void CheckForNewClients();

 protected:
  int                            m_port;
  int                            m_max_num_clients;
  SOCKET                         m_server_sd;
  SocketList                     m_client_sds;
};


class GestureServerOSC : public GestureServer {

 public:
  GestureServerOSC(string dest_ip, int dest_port=57120);
  GestureServerOSC(const GestureServerOSC& from);
  ~GestureServerOSC();
  GestureServerOSC& operator=(const GestureServerOSC& from);

  virtual void Start();
  virtual void Stop();
  virtual void Send(const HVState& state);

 protected:
  string                         m_dest_ip;
  int                            m_dest_port;
  SOCKET                         m_server_sd;
  mutable OSCpacket              m_packet;
};


