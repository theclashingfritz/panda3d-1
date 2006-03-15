// Filename: bamTextureMode.cxx
// Created by:  drose (14Mar06)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "bamTextureMode.h"
#include "config_util.h"

ostream &
operator << (ostream &out, BamTextureMode btm) {
  switch (btm) {
  case BTM_unchanged:
    return out << "unchanged";
   
  case BTM_fullpath:
    return out << "fullpath";
    
  case BTM_relative:
    return out << "relative";
    
  case BTM_basename:
    return out << "basename";

  case BTM_rawdata:
    return out << "rawdata";
  }

  return out << "**invalid BamTextureMode (" << (int)btm << ")**";
}

istream &
operator >> (istream &in, BamTextureMode &btm) {
  string word;
  in >> word;

  if (cmp_nocase(word, "unchanged") == 0) {
    btm = BTM_unchanged;
  } else if (cmp_nocase(word, "fullpath") == 0) {
    btm = BTM_fullpath;
  } else if (cmp_nocase(word, "relative") == 0) {
    btm = BTM_relative;
  } else if (cmp_nocase(word, "basename") == 0) {
    btm = BTM_basename;
  } else if (cmp_nocase(word, "rawdata") == 0) {
    btm = BTM_rawdata;

  } else {
    util_cat->error() << "Invalid BamTextureMode value: " << word << "\n";
    btm = BTM_relative;
  }

  return in;
}
