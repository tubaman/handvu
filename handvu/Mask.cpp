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
  * $Id: Mask.cpp,v 1.7 2005/08/16 00:14:26 matz Exp $
**/

#include "Common.h"
#include "Mask.h"
#include "Exceptions.h"
#include <fstream>

Mask::Mask()
: m_width(-1),
  m_height(-1),
  m_image_area_ratio(-1),
  m_name("")
{
}

Mask::~Mask()
{
}

void Mask::ParseFrom(string filename)
{
  ifstream mask_file(filename.c_str());
  if (!mask_file.is_open()) {
    throw HVEFileNotFound(filename);
  }

  string line;
  do {
    getline(mask_file, line);
  } while (line=="" || line[0]=='#');

  m_probs.clear();
  if (line==HV_MASK_VERSION_1_0_STRING) {
	  // version 1.0

    do {
      getline(mask_file, line);
    } while (line=="" || line[0]=='#');
    float ratio;
    char name[1024];
    int scanned = sscanf(line.c_str(), "Mask %s %dx%d, ratio %f", 
                         name, &m_width, &m_height, &ratio);
    if (scanned!=4) {
      throw HVEFile(filename, string("expected mask metadata, found: ")+line);
    }
    if (name[strlen(name)-1]==',') name[strlen(name)-1] = 0;
    if (name[strlen(name)-1]=='"') name[strlen(name)-1] = 0;
    if (name[0]=='"') {
      m_name = string(&name[1]);
    } else {
      m_name = string(name);
    }
    m_image_area_ratio = ratio;
    m_probs.resize(m_width*m_height);
    for (int row=0; row<m_height; row++) {
      do {
        getline(mask_file, line);
      } while (line=="" || line[0]=='#');
      char* cline = (char*) alloca(line.size()*sizeof(char));
      strcpy(cline, line.c_str());
      char* ctok = strtok(cline, " ");
      for (int col=0; col<m_width; col++) {
        double val = atof(ctok);
        m_probs[row*m_width+col] = val;
        ctok = strtok(NULL, " ");
        if (ctok==NULL && col<m_width-1) {
          throw HVEFile(filename, string("expected probabilities, found: ")+line);
        }
      }
    }

  } else {
    // wrong version
    throw HVEFile(filename, string("missing or wrong version: ")+line);
  }
  mask_file.close();
}

double Mask::GetProb(int x, int y) const
{
  ASSERT(0<=x && x<m_width);
  ASSERT(0<=y && y<m_height);
  return m_probs[y*m_width+x];
}
