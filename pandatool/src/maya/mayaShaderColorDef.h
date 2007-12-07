// Filename: mayaShaderColorDef.h
// Created by:  drose (12Apr03)
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

#ifndef MAYASHADERCOLORDEF_H
#define MAYASHADERCOLORDEF_H

#include "pandatoolbase.h"

#include "luse.h"
#include "lmatrix.h"

class MObject;
class MPlug;
class MayaShader;
class MayaShaderColorDef;
typedef pvector<MayaShaderColorDef *> MayaShaderColorList;
typedef pmap<string, string> MayaFileToUVSetMap;

////////////////////////////////////////////////////////////////////
//       Class : MayaShaderColorDef
// Description : This defines the various attributes that Maya may
//               associate with the "color" channel for a particular
//               shader (as well as on the "transparency" channel).
////////////////////////////////////////////////////////////////////
class MayaShaderColorDef {
public:
  MayaShaderColorDef();
  MayaShaderColorDef (MayaShaderColorDef&);
  ~MayaShaderColorDef();

  string strip_prefix(string full_name);
  
  LMatrix3d compute_texture_matrix() const;
  bool has_projection() const;
  TexCoordd project_uv(const LPoint3d &pos, const LPoint3d &ref_point) const;
  bool reset_maya_texture(const Filename &texture);
  
  void write(ostream &out) const;

  enum BlendType {
    BT_unspecified, 
    BT_modulate, 
    BT_decal,
    BT_blend,
    BT_replace,
    BT_add,
    BT_blend_color_scale,
    BT_modulate_glow_map,
    BT_normal_map,
    BT_gloss_map,
    BT_glow_map,
    BT_normal_gloss_map,
    BT_selector_map,
  };

  enum ProjectionType {
    PT_off,
    PT_planar,
    PT_spherical,
    PT_cylindrical,
    PT_ball,
    PT_cubic,
    PT_triplanar,
    PT_concentric,
    PT_perspective,
  };
  
  BlendType _blend_type;
  ProjectionType _projection_type;
  LMatrix4d _projection_matrix;
  double _u_angle;
  double _v_angle;
    
  Filename _texture_filename;
  string _texture_name;
  Colorf _color_gain;
  
  LVector2f _coverage;
  LVector2f _translate_frame;
  double _rotate_frame;
  
  bool _mirror;
  bool _stagger;
  bool _wrap_u;
  bool _wrap_v;

  LVector2f _repeat_uv;
  LVector2f _offset;
  double _rotate_uv;

  bool _is_alpha;
  
  string _uvset_name;
  MayaShaderColorDef *_opposite;
  
  string get_panda_uvset_name();

private:
  MObject *_color_object;
  
private:
  static void find_textures_modern(const string &shadername, MayaShaderColorList &list, MPlug inplug, bool is_alpha);
  void find_textures_legacy(MayaShader *shader, MObject color, bool trans=false);

  void set_projection_type(const string &type);

  LPoint2d map_planar(const LPoint3d &pos, const LPoint3d &centroid) const;
  LPoint2d map_spherical(const LPoint3d &pos, const LPoint3d &centroid) const;
  LPoint2d map_cylindrical(const LPoint3d &pos, const LPoint3d &centroid) const;

  // Define a pointer to one of the above member functions.
  LPoint2d (MayaShaderColorDef::*_map_uvs)(const LPoint3d &pos, const LPoint3d &centroid) const;
  
  friend class MayaShader;


  // Deprecated Fields - these fields are only used by the
  // deprecated codepath.  These fields are deprecated for the
  // following reasons:
  //
  // * has_texture is redundant --- if there's no
  // texture, just don't allocate a MayaShaderColorDef.
  //
  // * has_flat_color and flat_color don't belong here,
  // they belong in the shader. 
  //
  // * has_alpha_channel is not needed - there are better
  // ways to determine if a texture stage involves an alpha
  // channel.
  //
  // * keep_color, keep_alpha, and interpolate are all
  // adjuncts to blend_mode - it would make more sense just to
  // add some more blend_modes.  

public:
  bool     _has_texture;       // deprecated, see above.
  bool     _has_flat_color;    // deprecated, see above.
  Colord   _flat_color;        // deprecated, see above.
  bool     _has_alpha_channel; // deprecated, see above.
  bool     _keep_color;        // deprecated, see above.
  bool     _keep_alpha;        // deprecated, see above.
  bool     _interpolate;       // deprecated, see above.

};

#endif

