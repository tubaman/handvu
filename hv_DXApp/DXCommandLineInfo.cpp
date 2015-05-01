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
  * $Id: DXCommandLineInfo.cpp,v 1.4 2004/11/24 08:38:40 matz Exp $
**/

#include "stdafx.h"
#include "DXCommandLineInfo.h"

CDXCommandLineInfo::CDXCommandLineInfo()
{
  // $IT_DATA environmet variable, NULL if not set
  //const char *it_data = getenv("IT_DATA");
  //m_conductor_filename = string(it_data)+"\\hmd.conductor";
  //m_conductor_filename = string(it_data)+"\\intheworks.conductor";
  m_conductor_filename = "default.conductor";
  m_log_filename = "";
  m_verbosity = 0;
  m_print_version = false;
}

void CDXCommandLineInfo::ParseParam(const char* pszParam, BOOL bFlag, BOOL bLast)
{
  if (bFlag && pszParam[0]=='c' && pszParam[1]=='=') {
    m_conductor_filename = string(&pszParam[2]);
  } else if (bFlag && pszParam[0]=='l' && pszParam[1]=='=') {
    m_log_filename = string(&pszParam[2]);
  } else if (bFlag && strcmp(pszParam, "version")==0) {
    m_print_version = true;
  } else if (bFlag && pszParam[0]=='v' && pszParam[1]=='=') {
    m_verbosity = atoi(&pszParam[2]);
  } else {
    CCommandLineInfo::ParseParam(pszParam, bFlag, bLast);
  }
}

const string& CDXCommandLineInfo::GetConductor() const
{
  return m_conductor_filename;
}

const string& CDXCommandLineInfo::GetLogFilename() const
{
  return m_log_filename;
}

