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
  * $Id: DXCommandLineInfo.h,v 1.4 2004/11/24 08:38:40 matz Exp $
**/

#include <string>
using namespace std;

class CDXCommandLineInfo : public CCommandLineInfo
{
public:
  CDXCommandLineInfo();

  virtual void ParseParam(
    const char* pszParam,
    BOOL bFlag,
    BOOL bLast
  );

  const string& GetConductor() const;
  const string& GetLogFilename() const;
  int GetVerbosity() const {return m_verbosity;};
  bool PrintVersion() const {return m_print_version;}
  
protected:
  string       m_conductor_filename;
  string       m_log_filename;
  int          m_verbosity;
  bool         m_print_version;
};

