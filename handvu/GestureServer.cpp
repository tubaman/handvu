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
  * $Id: GestureServer.cpp,v 1.8 2006/01/03 21:44:15 matz Exp $
**/

// GestureServer.cpp: TCP/IP and UDP server
//


#include "Common.h"
#include "GestureServer.h"
#include "Exceptions.h"
#include "OSCpacket.h"

#include <sys/types.h>
#include <errno.h>

#ifndef WIN32

#if !defined(HAVE_ARPA_INET_H) || !defined(HAVE_NETINET_IN_H) || !defined(HAVE_FCNTL_H) || !defined(HAVE_SYS_SOCKET_H) || !defined(HAVE_SOCKET) || !defined(HAVE_STRERROR)
#error need socket interfaces
#endif

#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#endif


#ifdef USE_MFC
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif // _DEBUG
#endif // USE_MFC


/////////////////////////////////////////////////////////////////////////////
// GestureServer

GestureServer::GestureServer()
  : m_started(false)
{
}

GestureServer& GestureServer::operator=(const GestureServer&)
{
  ASSERT(0);  // must be overidden
  return *this;
}

void GestureServer::SetNonBlocking(SOCKET sd)
{
#ifdef WIN32
  u_long flags = 1;
  int error = 
    ioctlsocket(sd, FIONBIO, &flags);
  if (error!=0) {
    //    int errcode = WSAGetLastError();
    throw HVException("can not set socket to non-blocking");
  }
  
#else
  
  int error = 
    fcntl(sd, F_SETFL, O_NONBLOCK);
  if (error!=0) {
    throw HVException(string("can not set server socket to non-blocking: ")
		      + strerror(errno));
  }
#endif
}




/////////////////////////////////////////////////////////////////////////////
// GestureServerStream

GestureServerStream::GestureServerStream(int port, int max_num_clients)
  : GestureServer(),
    m_port(port),
    m_max_num_clients(max_num_clients),
    m_server_sd(0)
{
}

GestureServerStream::GestureServerStream(const GestureServerStream& from)
  : GestureServer(),
    m_port(from.m_port),
    m_max_num_clients(from.m_max_num_clients),
    m_server_sd(0)
{
  if (from.m_started) {
    throw HVException("can not copy this GestureServer - it has been started");
  }
}

GestureServerStream::~GestureServerStream()
{
  if (m_started) {
    Stop();
  }
}

GestureServerStream& GestureServerStream::operator=(const GestureServerStream& from)
{
  if (m_started) {
    VERBOSE0(5, "GestureServerStream::operator=: warning: stopping *this!");
    Stop();
  }
  if (from.m_started) {
    throw HVException("can not copy this GestureServerStream - it has been started");
  }
  m_port = from.m_port;
  m_max_num_clients = from.m_max_num_clients;
  m_server_sd = 0;
  m_started = false;
  return *this;
}

void GestureServerStream::Start()
{
  if (m_started) {
    throw HVException("GestureServerStream has already been started");
  }
  
  InitSockets();

  // create, bind, listen socket, set to non-blocking
  m_server_sd = socket(PF_INET, SOCK_STREAM, 0);
#ifdef WIN32
  if (m_server_sd==INVALID_SOCKET || m_server_sd==SOCKET_ERROR) {
    int errval = WSAGetLastError();
    throw HVException(string("error during socket creation: ") 
                      + strerror(errval));
  }
#else
  if (m_server_sd==-1) {
    throw HVException(string("error during socket creation: ") 
                      + strerror(errno));
  }
#endif

  // non-blocking
  SetNonBlocking(m_server_sd);

  // bind socket
  struct sockaddr_in server_sock_addr;
  memset(&server_sock_addr, 0, sizeof(server_sock_addr));
  server_sock_addr.sin_family = AF_INET;
  server_sock_addr.sin_addr.s_addr = INADDR_ANY;
  server_sock_addr.sin_port = htons((unsigned short) m_port);
  int error =
    bind(m_server_sd, (struct sockaddr*) &server_sock_addr, 
	 sizeof(server_sock_addr));
  if (error!=0) {
    char buf[256];
    sprintf(buf, "%d", m_port);
    string err = string("GestureServer can not bind socket to port ") + buf;
    throw HVException(err);
  }

  // listen with socket
  error = listen(m_server_sd, m_max_num_clients);
  if (error!=0) {
    throw HVException("can not listen on socket");
  }

  m_started = true;
}

void GestureServerStream::Stop()
{
  if (!m_started) {
    throw HVException("GestureServer had not been started");
  }

#ifdef WIN32
  closesocket(m_server_sd);
#else
  close(m_server_sd);
#endif
  for (SocketList::iterator it = m_client_sds.begin();
       it != m_client_sds.end();
       it ++) {
#ifdef WIN32
    closesocket(*it);
#else
    close(*it);
#endif
  }

  m_started = false;
}

void GestureServerStream::Send(const HVState& state)
{
  if (!m_started) {
    throw HVException("GestureServer has not been started");
  }
  
  // check for new clients
  CheckForNewClients();
  
  string str;
  char* buf = (char*) alloca(256+state.m_posture.size());
  float orientation = 0; 
  sprintf(buf, GESTUREEVENT_VERSION_STRING
	  " %lld %d: %d, %d, \"%s\" (%f, %f) [%f, %f]\r\n",
#ifdef WIN32
	  (long) state.m_tstamp,
#else
	  state.m_tstamp,
#endif
	  state.m_obj_id, (int) state.m_tracked,
	  (int) state.m_recognized, 
	  state.m_posture.c_str(), 
	  (float) state.m_center_xpos, (float) state.m_center_ypos,
	  (float) state.m_scale, orientation);
  str = string(buf);
  
  SocketList::iterator deleteme = m_client_sds.end();
  for (SocketList::iterator it = m_client_sds.begin();
       it != m_client_sds.end();
       it ++) {
    if (deleteme!=m_client_sds.end()) {
      m_client_sds.erase(deleteme);
      deleteme = m_client_sds.end();
    }
    int flags = 0;
#ifndef WIN32
    flags = MSG_NOSIGNAL;
#endif
    int bytes = send(*it, str.c_str(), (int)str.size()+1, flags);
    
    if (bytes==(int)str.size()+1) {
      // all went fine
      
#ifdef WIN32
    } else if (bytes==SOCKET_ERROR) {
      int errval = WSAGetLastError();
      if (errval==EPIPE 
          || errval==WSAESHUTDOWN || errval==WSAEHOSTUNREACH
          || errval==WSAECONNABORTED || errval==WSAECONNRESET 
          || errval==WSAETIMEDOUT)
#else
    } else if (bytes==-1) {
      if (errno==EPIPE 
          || errno==ESHUTDOWN || errno==EHOSTUNREACH
          || errno==ECONNABORTED || errno==ECONNRESET 
          || errno==ETIMEDOUT)
#endif
      {
        // broken connection - throw out this client
        VERBOSE0(2, "GestureServer::Send(): "
		 "broken connection, discarding client");
        deleteme = it;
	
      } else {
        VERBOSE1(2, "warning: ignoring error in GestureServer::Send(): %s",
		 strerror(errno));
	
        // EAGAIN: data didn't fit into the buffer
        // EMSGSIZE: message size waaaay too big
      }
      
    } else if (bytes>0 && bytes != (int)str.size()+1) {
      VERBOSE0(2, "warning: ignoring error in GestureServer::Send(): "
	       "not all bytes of message were sent");
      
    } else {
      VERBOSE0(2, "warning: ignoring unknown error in GestureServer::Send()");
    }
  }
  if (deleteme!=m_client_sds.end()) {
    m_client_sds.erase(deleteme);
  }
}

void GestureServerStream::CheckForNewClients()
{
  struct sockaddr_in client_addr;
#ifdef WIN32
  int client_sock_len = sizeof(struct sockaddr);
#else
  socklen_t client_sock_len = (socklen_t) sizeof(struct sockaddr);
#endif
  SOCKET client_sd = accept(m_server_sd,
			    (struct sockaddr*) &client_addr, &client_sock_len);
#ifdef WIN32
  if (client_sd!=SOCKET_ERROR)
#else
  if (client_sd!=-1)
#endif
  {
    // a new client is requesting connection

#ifdef HAVE_INET_NTOA
    VERBOSE2(3, "client connected on port %d from IP %s",
	     m_port, inet_ntoa(client_addr.sin_addr));
#else
    VERBOSE1(3, "client connected on port %d", m_port);
#endif
    
    // set non-blocking
    SetNonBlocking(client_sd);
    
    m_client_sds.push_back(client_sd);
  }
}

void GestureServerStream::InitSockets()
{
#ifdef WIN32
  static bool WSAInitialized = false;
  if (!WSAInitialized) {
    WORD wVersionRequested;
    WSADATA wsaData;
    
    wVersionRequested = MAKEWORD(2, 2);
    
    int err = WSAStartup(wVersionRequested, &wsaData);
    if (err) {
      throw HVException("could not find usable WinSock DLL");
    }
    
    /* Confirm that the WinSock DLL supports 2.2.*/
    /* Note that if the DLL supports versions greater    */
    /* than 2.2 in addition to 2.2, it will still return */
    /* 2.2 in wVersion since that is the version we      */
    /* requested.                                        */
    
    if ( LOBYTE( wsaData.wVersion ) != 2 ||
	 HIBYTE( wsaData.wVersion ) != 2 ) {
      WSACleanup();
      throw HVException("could not find WinSock DLL >= 2.2");
    }
    WSAInitialized = true;
  }
#endif
}









/////////////////////////////////////////////////////////////////////////////
// GestureServerOSC

GestureServerOSC::GestureServerOSC(string dest_ip, int dest_port)
  : GestureServer(),
    m_dest_ip(dest_ip),
    m_dest_port(dest_port),
    m_server_sd(0)
{
}

GestureServerOSC::GestureServerOSC(const GestureServerOSC& from)
  : GestureServer(),
    m_dest_ip(from.m_dest_ip),
    m_dest_port(from.m_dest_port),
    m_server_sd(0)
{
  if (from.m_started) {
    throw HVException("can not copy this GestureServer - it has been started");
  }
}

GestureServerOSC::~GestureServerOSC()
{
  if (m_started) {
    Stop();
  }
}

GestureServerOSC& GestureServerOSC::operator=(const GestureServerOSC& from)
{
  if (m_started) {
    VERBOSE0(5, "GestureServerOSC::operator=: warning: stopping *this!");
    Stop();
  }
  if (from.m_started) {
    throw HVException("can not copy this GestureServerOSC - it has been started");
  }
  m_dest_ip = from.m_dest_ip;
  m_dest_port = from.m_dest_port;
  m_server_sd = from.m_server_sd;
  m_started = false;
  return *this;
}

void GestureServerOSC::Start()
{
  if (m_started) {
    throw HVException("GestureServerOSC has already been started");
  }
  
  // create socket, set to non-blocking
  m_server_sd = socket(PF_INET, SOCK_DGRAM, 0);
#ifdef WIN32
  if (m_server_sd==INVALID_SOCKET || m_server_sd==SOCKET_ERROR) {
    int errval = WSAGetLastError();
    throw HVException(string("error during socket creation: ") 
                      + strerror(errval));
  }
#else
  if (m_server_sd==-1) {
    throw HVException(string("error during socket creation: ") 
                      + strerror(errno));
  }
#endif

  // non-blocking
  SetNonBlocking(m_server_sd);

  m_started = true;
}

void GestureServerOSC::Stop()
{
  if (!m_started) {
    throw HVException("GestureServer had not been started");
  }

#ifdef WIN32
  closesocket(m_server_sd);
#else
  close(m_server_sd);
#endif
  
  m_started = false;
}

void GestureServerOSC::Send(const HVState& state)
{
  if (!m_started) {
    throw HVException("GestureServer has not been started");
  }

  m_packet.Clear();
  m_packet.SetAddress("/gesture_event");
  m_packet.AddInt((int) state.m_tstamp);
  m_packet.AddInt(state.m_obj_id);
  m_packet.AddInt(state.m_tracked);
  m_packet.AddInt(state.m_recognized);
  m_packet.AddString(state.m_posture);
  m_packet.AddFloat((float)state.m_center_xpos);
  m_packet.AddFloat((float)state.m_center_ypos);
  m_packet.AddFloat((float)state.m_scale);
  float orientation = 0.0;
  m_packet.AddFloat(orientation);

  int flags = 0;
  struct sockaddr_in dest_sock_addr;
  memset(&dest_sock_addr, 0, sizeof(dest_sock_addr));
  dest_sock_addr.sin_family = AF_INET;
#ifdef WIN32
  dest_sock_addr.sin_addr.S_un.S_addr = inet_addr(m_dest_ip.c_str());
  if (dest_sock_addr.sin_addr.S_un.S_addr == INADDR_NONE)
#else
  int success = inet_aton(m_dest_ip.c_str(), &dest_sock_addr.sin_addr);
  if (!success)
#endif
  {
    throw HVException(string("invalid address: ") + m_dest_ip);
  }
  dest_sock_addr.sin_port = htons((unsigned short) m_dest_port);

  int sent = sendto(m_server_sd, m_packet.GetBytes(), m_packet.GetSize(), flags,
		    (sockaddr*) &dest_sock_addr, sizeof(dest_sock_addr));

#ifdef WIN32
  if (sent==SOCKET_ERROR) {
    int last_error = GetLastError();
    char msg[250];
    sprintf(msg, "error sending OSC message (errno %d)", last_error);
    throw HVException(msg);
  }
#else
  if (sent==-1) {
    throw HVException(string("error sending OSC message: ") +
		      strerror(errno));
  }
#endif
}


