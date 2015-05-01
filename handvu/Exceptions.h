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
  * $Id: Exceptions.h,v 1.6 2004/11/11 01:58:12 matz Exp $
**/

#if !defined(__HVEXCEPTIONS_H__INCLUDED_)
#define __HVEXCEPTIONS_H__INCLUDED_

#include <string>
using namespace std;

#if defined(WIN32) && defined(GetMessage)
#undef GetMessage
#endif

class HVException {
public:
  HVException(string msg);
  virtual ~HVException() {};
  virtual string GetMessage() const;
  virtual ostream& output(ostream& os) const;
  
  friend ostream& operator<<(ostream& os, const HVException& ite);

 protected:
  string m_msg;
};


class HVEFile : public HVException {
 public:
  HVEFile(string filename, string msg);
  virtual ~HVEFile() {};

 protected:
  string m_filename;
};

class HVEFileNotFound : public HVEFile {
 public:
  virtual ~HVEFileNotFound() {};
  HVEFileNotFound(string filename);

};

#endif // __HVEXCEPTIONS_H__INCLUDED_


