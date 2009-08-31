// Filename: loaderOptions.cxx
// Created by:  drose (05Oct05)
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

#include "loaderOptions.h"
#include "indent.h"

////////////////////////////////////////////////////////////////////
//     Function: LoaderOptions::output
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void LoaderOptions::
output(ostream &out) const {
  out << "LoaderOptions(";
  string sep = "";
  write_flag(out, sep, "LF_search", LF_search);
  write_flag(out, sep, "LF_report_errors", LF_report_errors);
  if ((_flags & LF_convert_anim) == LF_convert_anim) {
    write_flag(out, sep, "LF_convert_anim", LF_convert_anim);
  } else {
    write_flag(out, sep, "LF_convert_skeleton", LF_convert_skeleton);
    write_flag(out, sep, "LF_convert_channels", LF_convert_channels);
  }
  if ((_flags & LF_no_cache) == LF_no_cache) {
    write_flag(out, sep, "LF_no_cache", LF_no_cache);
  } else {
    write_flag(out, sep, "LF_no_disk_cache", LF_no_disk_cache);
    write_flag(out, sep, "LF_no_ram_cache", LF_no_ram_cache);
  }
  if (sep.empty()) {
    out << "0";
  }
  out << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderOptions::write_flag
//       Access: Private
//  Description: Used to implement output().
////////////////////////////////////////////////////////////////////
void LoaderOptions::
write_flag(ostream &out, string &sep, 
           const string &flag_name, int flag) const {
  if ((_flags & flag) == flag) {
    out << sep << flag_name;
    sep = " | ";
  }
}