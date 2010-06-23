// Filename: colladaSource.h
// Created by: Xidram (20Apr10)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef COLLADASOURCE_H
#define COLLADASOURCE_H

#include "colladaAccessor.h"
#include "colladaArray.h"
#include "colladaAssetElement.h"
#include "typedReferenceCount.h"

////////////////////////////////////////////////////////////////////
//       Class : ColladaSource
// Description : Object that represents the <source> COLLADA element.
////////////////////////////////////////////////////////////////////
class EXPCL_COLLADA ColladaSource : public ColladaAssetElement {
PUBLISHED:
  ColladaSource();
  virtual ~ColladaSource() {};

  INLINE bool has_array() const;
  INLINE PT(ColladaArrayBase) get_array() const;
  INLINE void set_array(ColladaArrayBase* array);
  INLINE void clear_array();

  INLINE ColladaArrayType get_array_type() const;

  INLINE PT(ColladaAccessor) get_accessor() const;
  INLINE bool get_values(PTA_LVecBase4f &into) const;

  virtual void clear();
  virtual bool load_xml(const TiXmlElement *xelement);
  virtual TiXmlElement *make_xml() const;
  virtual PT(ColladaElement) get_element_by_id(const string &id) const;

protected:
  PT(ColladaArrayBase) _array;
  PT(ColladaAccessor) _accessor;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ColladaAssetElement::init_type();
    register_type(_type_handle, "ColladaSource",
                  ColladaAssetElement::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "colladaSource.I"

#endif

