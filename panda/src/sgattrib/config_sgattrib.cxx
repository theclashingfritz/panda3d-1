// Filename: config_sgattrib.cxx
// Created by:  drose (10Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "config_sgattrib.h"
#include "renderRelation.h"
#include "colorTransition.h"
#include "colorAttribute.h"
#include "textureTransition.h"
#include "textureAttribute.h"
#include "decalTransition.h"
#include "decalAttribute.h"
#include "depthTestTransition.h"
#include "depthTestAttribute.h"
#include "depthWriteTransition.h"
#include "depthWriteAttribute.h"
#include "colorBlendTransition.h"
#include "colorBlendAttribute.h"
#include "renderModeTransition.h"
#include "renderModeAttribute.h"
#include "materialTransition.h"
#include "materialAttribute.h"
#include "texGenTransition.h"
#include "texGenAttribute.h"
#include "cullFaceTransition.h"
#include "cullFaceAttribute.h"
#include "colorMaskTransition.h"
#include "colorMaskAttribute.h"
#include "stencilTransition.h"
#include "stencilAttribute.h"
#include "textureApplyTransition.h"
#include "textureApplyAttribute.h"
#include "clipPlaneTransition.h"
#include "clipPlaneAttribute.h"
#include "transparencyTransition.h"
#include "transparencyAttribute.h"
#include "fogTransition.h"
#include "fogAttribute.h"
#include "linesmoothTransition.h"
#include "linesmoothAttribute.h"
#include "transformTransition.h"
#include "transformAttribute.h"
#include "texMatrixTransition.h"
#include "texMatrixAttribute.h"
#include "billboardTransition.h"
#include "showHideTransition.h"
#include "showHideAttribute.h"
#include "polygonOffsetTransition.h"
#include "polygonOffsetAttribute.h"
#include "pruneTransition.h"
#include "drawBoundsTransition.h"
#include "pointShapeTransition.h"
#include "pointShapeAttribute.h"
#include "colorMatrixAttribute.h"
#include "colorMatrixTransition.h"
#include "alphaTransformAttribute.h"
#include "alphaTransformTransition.h"


#include <dconfig.h>

Configure(config_sgattrib);
NotifyCategoryDef(sgattrib, "");


// For performance testing reasons, it may be useful to support decals
// (specially rendered coplanar geometry) to varying
// less-than-complete degrees.  Modify the variable support-decals to
// change this.  The legal values are:
//
//   on - This is the default, and causes decals to be rendered
//        properly (if supported by the gsg backend).  This could have
//        performance implications in fill, transform, and
//        state-sorting.  This is equivalent to #t.
//
//  off - Decals are rendered as if they were not decalled at all.
//        The result will generally be horrible looking, with each
//        decal Z-fighting with its base.  This is equivalent to #f.
//
// hide - Decals are not drawn at all.
//
// If compiled in NDEBUG mode, this variable is ignored and decals are
// always on.
//
SupportDecals support_decals = SD_on;

static SupportDecals
parse_support_decals(const string &type) {
  if (type == "on") {
    return SD_on;
  } else if (type == "off") {
    return SD_off;
  } else if (type == "hide") {
    return SD_hide;
  }
  return SD_invalid;
}

ConfigureFn(config_sgattrib) {
  string support_decals_str = config_sgattrib.GetString("support-decals", "");
  if (!support_decals_str.empty()) {
    support_decals = parse_support_decals(support_decals_str);
    if (support_decals == SD_invalid) {
      support_decals = 
	config_sgattrib.GetBool("support-decals", true) ? SD_on : SD_off;
    }
  }

  // MPG - we want to ensure that texture transitions are applied
  // before texgen transitions, so the texture transition must be
  // initialized first.

  RenderRelation::init_type();
  TextureTransition::init_type();
  TextureAttribute::init_type();
  TransformTransition::init_type();
  TransformAttribute::init_type();
  TexMatrixTransition::init_type();
  TexMatrixAttribute::init_type();
  DecalTransition::init_type();
  DecalAttribute::init_type();
  DepthTestTransition::init_type();
  DepthTestAttribute::init_type();
  DepthWriteTransition::init_type();
  DepthWriteAttribute::init_type();
  ColorBlendTransition::init_type();
  ColorBlendAttribute::init_type();
  RenderModeTransition::init_type();
  RenderModeAttribute::init_type();
  MaterialTransition::init_type();
  MaterialAttribute::init_type();
  TexGenTransition::init_type();
  TexGenAttribute::init_type();
  CullFaceTransition::init_type();
  CullFaceAttribute::init_type();
  ColorMaskTransition::init_type();
  ColorMaskAttribute::init_type();
  StencilTransition::init_type();
  StencilAttribute::init_type();
  TextureApplyTransition::init_type();
  TextureApplyAttribute::init_type();
  ClipPlaneTransition::init_type();
  ClipPlaneAttribute::init_type();
  TransparencyTransition::init_type();
  TransparencyAttribute::init_type();
  FogTransition::init_type();
  FogAttribute::init_type();
  LinesmoothTransition::init_type();
  LinesmoothAttribute::init_type();
  ShowHideTransition::init_type();
  ShowHideAttribute::init_type();
  PruneTransition::init_type();
  ColorTransition::init_type();
  ColorAttribute::init_type();
  BillboardTransition::init_type();
  DrawBoundsTransition::init_type();
  PointShapeTransition::init_type();
  PointShapeAttribute::init_type();
  PolygonOffsetTransition::init_type();
  PolygonOffsetAttribute::init_type();
  ColorMatrixTransition::init_type();
  ColorMatrixAttribute::init_type();
  AlphaTransformTransition::init_type();
  AlphaTransformAttribute::init_type();

  // Register the RenderRelation class for the
  // NodeRelation::create_typed_arc() function.
  RenderRelation::register_with_factory();

  //Registration of writeable object's creation
  //functions with BamReader's factory
  RenderRelation::register_with_read_factory();
  TransformTransition::register_with_read_factory();
  TextureTransition::register_with_read_factory();
  TextureApplyTransition::register_with_read_factory();
  TransparencyTransition::register_with_read_factory();
  CullFaceTransition::register_with_read_factory();
  DecalTransition::register_with_read_factory();
  PruneTransition::register_with_read_factory();
  BillboardTransition::register_with_read_factory();
  ColorMatrixTransition::register_with_read_factory();
}

