// Filename: cvsSourceTree.h
// Created by:  drose (31Oct00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef CVSSOURCETREE_H
#define CVSSOURCETREE_H

#include <pandatoolbase.h>

#include "pvector.h"
#include "pmap.h"

class CVSSourceDirectory;

////////////////////////////////////////////////////////////////////
//       Class : CVSSourceTree
// Description : This represents the root of the tree of source
//               directory files.
////////////////////////////////////////////////////////////////////
class CVSSourceTree {
public:
  CVSSourceTree();
  ~CVSSourceTree();

  void set_root(const string &root_path);
  bool scan(const string &key_filename);

  CVSSourceDirectory *get_root() const;
  CVSSourceDirectory *find_directory(const string &path);
  CVSSourceDirectory *find_relpath(const string &relpath);
  CVSSourceDirectory *find_dirname(const string &dirname);

  CVSSourceDirectory *choose_directory(const string &filename,
                                       CVSSourceDirectory *suggested_dir,
                                       bool force, bool interactive);

  string get_root_fullpath();
  string get_root_dirname() const;

  static bool temp_chdir(const string &path);
  static void restore_cwd();

public:
  void add_file(const string &filename, CVSSourceDirectory *dir);

private:
  typedef pvector<CVSSourceDirectory *> Directories;

  CVSSourceDirectory *
  prompt_user(const string &filename, CVSSourceDirectory *suggested_dir,
              const Directories &dirs, bool force, bool interactive);

  CVSSourceDirectory *ask_existing(const string &filename,
                                   CVSSourceDirectory *dir);
  CVSSourceDirectory *ask_existing(const string &filename,
                                   const Directories &dirs,
                                   CVSSourceDirectory *suggested_dir);
  CVSSourceDirectory *ask_new(const string &filename, CVSSourceDirectory *dir);
  CVSSourceDirectory *ask_any(const string &filename);

  string prompt(const string &message);

  static string get_actual_fullpath(const string &path);
  static string get_start_fullpath();

private:
  string _path;
  CVSSourceDirectory *_root;

  typedef pmap<string, Directories> Filenames;
  Filenames _filenames;

  static bool _got_start_fullpath;
  static string _start_fullpath;
  bool _got_root_fullpath;
  string _root_fullpath;
};

#endif
