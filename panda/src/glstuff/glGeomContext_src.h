// Filename: glGeomContext_src.h
// Created by:  drose (19Mar04)
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

#include "pandabase.h"
#include "geomContext.h"
#include "qpgeomMunger.h"
#include "qpgeomVertexData.h"
#include "pointerTo.h"
#include "pmap.h"

class CLP(GeomMunger);

////////////////////////////////////////////////////////////////////
//       Class : GLGeomContext
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_GL CLP(GeomContext) : public GeomContext {
public:
  INLINE CLP(GeomContext)(Geom *geom);
  virtual ~CLP(GeomContext)();

  bool get_display_list(GLuint &index, const CLP(GeomMunger) *munger, 
                        UpdateSeq modified);
  void release_display_lists();

  void remove_munger(CLP(GeomMunger) *munger);

  // This is used only for the old Geom interface.
  GLuint _deprecated_index;

  // The different variants of the display list, for storing the
  // different states the geom might have been rendered in (each using
  // a different munger).
  class DisplayList {
  public:
    INLINE DisplayList();
    GLuint _index;
    UpdateSeq _modified;
  };
  typedef pmap<CLP(GeomMunger) *, DisplayList> DisplayLists;
  DisplayLists _display_lists;

  // The number of vertices encoded in the display list, for stats
  // reporting.
#ifdef DO_PSTATS
  int _num_verts;
#endif

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GeomContext::init_type();
    register_type(_type_handle, CLASSPREFIX_QUOTED "GeomContext",
                  GeomContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "glGeomContext_src.I"

