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
  * $Id: Thread.h,v 1.4 2006/01/03 19:36:42 matz Exp $
**/

#ifndef __THREAD__INCLUDED_H_
#define __THREAD__INCLUDED_H_

#if defined(WIN32)
#include <windows.h>
#include <process.h>
//#include <afxwin.h>
//#include <afxmt.h>
#else //WIN32
#include <pthread.h>
#endif //WIN32

// ----------------------------------------------------------------------
// class Thread
// ----------------------------------------------------------------------

class Thread {
 public:
  Thread(void* (*fun)(void*), void* arg);
  void Start();
  void Stop();
  void Join();
  void Suspend(); // automatically releases all held locks
  void Resume();

  void Lock();
  void Unlock();

 protected:
  ~Thread();
  void*                 (*m_fun)(void*);
  void*                   m_arg;
#if defined(WIN32)
  uintptr_t               m_thread;
  HANDLE                  m_mutex;
  //CWinThread*             m_pThread;
  //CCriticalSection	    m_lock;
#else //WIN32
  pthread_t               m_threadID;
  pthread_mutex_t         m_mutex;
  pthread_cond_t          m_cond;
#endif //WIN32
};

#endif // __THREAD__INCLUDED_H_


