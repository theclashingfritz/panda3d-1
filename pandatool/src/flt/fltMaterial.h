// Filename: fltMaterial.h
// Created by:  drose (25Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef FLTMATERIAL_H
#define FLTMATERIAL_H

#include <pandatoolbase.h>

#include "fltRecord.h"

#include <luse.h>

class DatagramIterator;

////////////////////////////////////////////////////////////////////
// 	 Class : FltMaterial
// Description : Represents a single material in the material palette.
////////////////////////////////////////////////////////////////////
class FltMaterial : public FltRecord {
public:
  FltMaterial(FltHeader *header);

  enum Flags {
    F_materials_used    = 0x80000000,
  };

  int _material_index;
  string _material_name;
  unsigned int _flags;
  RGBColorf _ambient;
  RGBColorf _diffuse;
  RGBColorf _specular;
  RGBColorf _emissive;
  float _shininess;
  float _alpha;

protected:
  virtual bool extract_record(FltRecordReader &reader);
  virtual bool build_record(FltRecordWriter &writer) const;

public:
  bool extract_14_record(int index, DatagramIterator &di);

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    FltRecord::init_type();
    register_type(_type_handle, "FltMaterial",
		  FltRecord::get_class_type());
  }

private:
  static TypeHandle _type_handle;

  friend class FltHeader;
};

#endif


