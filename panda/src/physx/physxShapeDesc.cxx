// Filename: physxShapeDesc.cxx
// Created by:  enn0x (08Sep09)
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

#include "physxShapeDesc.h"
#include "physxManager.h"

TypeHandle PhysxShapeDesc::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PhysxShapeDesc::set_name
//       Access: Published
//  Description: Sets a possible debug name.
////////////////////////////////////////////////////////////////////
void PhysxShapeDesc::
set_name(const char *name) {

  ptr()->name = name;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxShapeDesc::set_trigger
//       Access: Published
//  Description: This shape will become a trigger shape if this
//               parameter is set to TRUE. It won't take part in
//               collisions, but trigger events if some other
//               shape passes through it.
////////////////////////////////////////////////////////////////////
void PhysxShapeDesc::
set_trigger(bool value) {

  if (value == true) {
    ptr()->shapeFlags |= NX_TRIGGER_ENABLE;
  } else {
    ptr()->shapeFlags &= ~(NX_TRIGGER_ENABLE);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxShapeDesc::set_local_pos
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxShapeDesc::
set_local_pos(const LPoint3f &pos) {

  ptr()->localPose.t = PhysxManager::point3_to_nxVec3(pos);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxShapeDesc::set_local_mat
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxShapeDesc::
set_local_mat(const LMatrix4f &mat) {

  ptr()->localPose = PhysxManager::mat4_to_nxMat34(mat);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxShapeDesc::set_local_hpr
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxShapeDesc::
set_local_hpr(float h, float p, float r) {

  LQuaternionf q;
  LMatrix3f rot;
  NxMat34 m;

  q.set_hpr(LVector3f(h, p, r));
  q.extract_to_matrix(rot);

  ptr()->localPose.M = PhysxManager::mat3_to_nxMat33(rot);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxShapeDesc::set_skin_width
//       Access: Published
//  Description: Specifies by how much shapes can interpenetrate. 
//
//               Two shapes will interpenetrate by the sum of their
//               skin widths. This means that their graphical
//               representations should be adjusted so that they
//               just touch when the shapes are interpenetrating.
//
//               The default skin width is the 'physx-skin-width'
//               parameter.
//
//               A skin width sum of zero for two bodies is not
//               permitted because it will lead to an unstable
//               simulation.
//
//               If your simulation jitters because resting bodies
//               occasionally lose contact, increasing the size of
//               your collision volumes and the skin width may
//               improve things.
////////////////////////////////////////////////////////////////////
void PhysxShapeDesc::
set_skin_width(float skinWidth) {

  nassertv(skinWidth >= 0.0f);
  ptr()->skinWidth = skinWidth;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxShapeDesc::set_flag
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxShapeDesc::
set_flag(const PhysxShapeFlag flag, bool value) {

  if (value == true) {
    ptr()->shapeFlags |= flag;
  } else {
    ptr()->shapeFlags &= ~(flag);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxShapeDesc::get_flag
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
bool PhysxShapeDesc::
get_flag(const PhysxShapeFlag flag) const {

  return (ptr()->shapeFlags & flag) ? true : false;
}
