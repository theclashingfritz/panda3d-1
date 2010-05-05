// Filename: config_collada.cxx
// Created by:  pro-rsoft (23Aug09)
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

#include "config_collada.h"

#include "dconfig.h"
#include "colladaAsset.h"
#include "colladaAssetElement.h"
#include "colladaCamera.h"
#include "colladaContributor.h"
#include "colladaDocument.h"
#include "colladaEffect.h"
#include "colladaElement.h"
#include "colladaGeometry.h"
#include "colladaLight.h"
#include "colladaMaterial.h"
#include "colladaMesh.h"
#include "colladaNode.h"
#include "colladaOptics.h"
#include "colladaPrimitive.h"
#include "colladaProfile.h"
#include "colladaRoot.h"
#include "colladaVisualScene.h"
#include "loaderFileTypeDae.h"
#include "loaderFileTypeRegistry.h"

ConfigureDef(config_collada);
NotifyCategoryDef(collada, "");

ConfigVariableBool collada_flatten
("collada-flatten", true,
 PRC_DESC("This is normally true to flatten out useless nodes after loading "
          "a COLLADA file.  Set it false if you want to see the complete "
          "and true hierarchy as the egg loader created it (although the "
          "extra nodes may have a small impact on render performance)."));

ConfigVariableDouble collada_flatten_radius
("collada-flatten-radius", 0.0,
 PRC_DESC("This specifies the minimum cull radius in the COLLADA file. "
          "Nodes whose bounding volume is smaller than this radius will "
          "be flattened tighter than nodes larger than this radius, to "
          "reduce the node count even further.  The idea is that small "
          "objects will not need to have their individual components "
          "culled separately, but large environments should.  This allows "
          "the user to specify what should be considered \"small\".  Set "
          "it to 0.0 to disable this feature."));

ConfigVariableBool collada_unify
("collada-unify", true,
 PRC_DESC("When this is true, then in addition to flattening the scene graph "
          "nodes, the COLLADA loader will also combine as many Geoms as "
          "possible within "
          "a given node into a single Geom.  This has theoretical performance "
          "benefits, especially on higher-end graphics cards, but it also "
          "slightly slows down egg loading."));

ConfigVariableBool collada_combine_geoms
("collada-combine-geoms", false,
 PRC_DESC("Set this true to combine sibling GeomNodes into a single GeomNode, "
          "when possible."));

ConfigVariableBool collada_accept_errors
("collada-accept-errors", true,
 PRC_DESC("When this is true, certain kinds of recoverable errors (not syntax "
          "errors) in a COLLADA file will be allowed and ignored when a "
          "COLLADA file is loaded.  When it is false, only perfectly "
          "pristine COLLADA files may be loaded."));

ConfigureFn(config_collada) {
  init_libcollada();
}

////////////////////////////////////////////////////////////////////
//     Function: init_libcollada
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libcollada() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  ColladaAsset::init_type();
  ColladaAssetElement::init_type();
  ColladaCamera::init_type();
  ColladaContributor::init_type();
  ColladaDocument::init_type();
  ColladaEffect::init_type();
  ColladaElement::init_type();
  ColladaGeometry::init_type();
  ColladaLight::init_type();
  ColladaMaterial::init_type();
  ColladaMesh::init_type();
  ColladaNode::init_type();
  ColladaOptics::init_type();
  ColladaPrimitive::init_type();
  ColladaProfile::init_type();
  ColladaRoot::init_type();
  ColladaVisualScene::init_type();
  LoaderFileTypeDae::init_type();

  LoaderFileTypeRegistry *reg = LoaderFileTypeRegistry::get_global_ptr();

  reg->register_type(new LoaderFileTypeDae);
}
