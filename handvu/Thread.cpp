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
  * $Id: Thread.cpp,v 1.4 2006/01/03 19:36:42 matz Exp $
**/

// Thread.cpp: wrapper around Afx and pthread
//

#include "Common.h"
#include "Thread.h"
#include "Exceptions.h"

#if defined(WIN32)
Thread::Thread(void* (*fun)(void*), void* arg)
{
  m_fun = fun;
  m_arg = arg;

  m_mutex = CreateMutex (NULL, FALSE, NULL);
  if (m_mutex==NULL) throw HVException("can not initialize WIN32 mutex");
}

Thread::~Thread()
{
}

/*
UINT AfxThreadFunConversion(LPVOID pParam)
{
  void** vp = (void**) pParam;
  void* (*fun)(void*) = (void* (*)(void*))vp[0];
  void* arg = vp[1];

  if (fun==NULL || arg==NULL) {
    return 1;   // if arguments are not valid
  }
  fun(arg);
    
  return 0;   // thread completed successfully
}
*/

void Thread::Start()
{
  m_thread = _beginthread((void (*)(void*))m_fun, 0, m_arg);
  if (m_thread == -1L) throw HVException("can not start WIN32 Thread");
//  _endthreadex( 0 ); in m_fun-wrapper??

 // void* vp[2];
 // vp[0] = (void*) m_fun;
 // vp[1] = m_arg;
	//m_pThread = AfxBeginThread(AfxThreadFunConversion, vp);
 // if (m_pThread==NULL) throw HVException("can not start Afx Thread");
}

void Thread::Stop()
{
  this->~Thread();
  //BOOL bDeleteFromMemory = TRUE;
  //AfxEndThread(0, bDeleteFromMemory);
}

void Thread::Join()
{
  DWORD result = WaitForSingleObject( m_mutex, INFINITE );
  if (result == WAIT_FAILED) throw HVException("can not join WIN32 thread");
}

// automatically releases all held locks
void Thread::Suspend()
{
  DWORD result = WaitForSingleObject( m_mutex, INFINITE );
  if (result == WAIT_FAILED) throw HVException("can not suspend WIN32 thread");
 //DWORD result = ::SuspendThread(GetCurrentThread());
  //if (result == 0xFFFFFFFF) throw HVException("can not suspend Afx Thread");
}

void Thread::Resume()
{
  BOOL success = ReleaseMutex(m_mutex);
  if (!success) throw HVException("can not resume WIN32 thread");
  //DWORD result = m_pThread->ResumeThread(); 
  //if (result == 0xFFFFFFFF) throw HVException("can not resume Afx Thread");
}

void Thread::Lock()
{
  DWORD result = WaitForSingleObject( m_mutex, INFINITE );
  if (result == WAIT_FAILED) throw HVException("can not lock Afx Lock");
}

void Thread::Unlock()
{
  BOOL success = ReleaseMutex(m_mutex);
  if (!success) throw HVException("can not unlock WIN32 Lock");
}

#else //WIN32
// now for Linux:

Thread::Thread(void* (*fun)(void*), void* arg)
{
  m_fun = fun;
  m_arg = arg;

  int err;
  err = pthread_cond_init(&m_cond, NULL);
  if (err) throw HVException("can not initialize pthread conditional variable"); 
  err = pthread_mutex_init(&m_mutex, NULL);
  if (err) throw HVException("can not initialize pthread mutex");
}

Thread::~Thread()
{
  int err;
  err = pthread_cond_destroy(&m_cond);
  if (err) throw HVException("can not destroy conditional variable"); 
  err = pthread_mutex_destroy(&m_mutex);
  if (err) throw HVException("can not destroy mutex");
}

void Thread::Start()
{
  int err = pthread_create(&m_threadID, NULL, m_fun, m_arg);
  if (err) throw HVException("can not start pthread");
}

void Thread::Stop()
{
  this->~Thread();
  pthread_exit(NULL);
}

void Thread::Join()
{
  int err = pthread_join(m_threadID, NULL);
  if (err) throw HVException("can not join pthread");
}

// automatically releases all held locks
void Thread::Suspend()
{
  ASSERT(mutex locked);
  int err = pthread_cond_wait(&m_cond, &m_mutex);
  if (err) throw HVException("can not wait on pthread conditional");
}

void Thread::Resume()
{
  //signal conditional variable;
  int err = pthread_cond_signal(&m_cond);
  if (err) throw HVException("can not signal pthread conditional");
}

void Thread::Lock()
{
  int err = pthread_mutex_lock(&m_mutex);
  if (err) throw HVException("can not lock pthread mutex");
}

void Thread::Unlock()
{
  int err = pthread_mutex_unlock(&m_mutex);
  if (err) throw HVException("can not unlock pthread mutex");
}

#endif //WIN32
