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
  * $Id: Exceptions.cpp,v 1.6 2004/11/11 01:58:12 matz Exp $
**/

#include "Common.h"
#include "Exceptions.h"
#include <ostream>

HVException::HVException(string msg)
{
  m_msg = msg;
}

string HVException::GetMessage() const
{
  printf("%s\n", m_msg.c_str());
  return m_msg;
}

ostream& HVException::output(ostream& os) const
{
  return os << m_msg;
}

ostream& operator<<(ostream& os, const HVException& ite) {
  return ite.output(os);
}


HVEFile::HVEFile(string filename, string msg)
  : HVException("error with file "+filename+":\n"+msg),
    m_filename(filename)
{
}


HVEFileNotFound::HVEFileNotFound(string filename)
  : HVEFile(filename, "could not find or open file")
{
}




