// Filename: physxPlaneShape.cxx
// Created by:  enn0x (16Sep09)
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

#include "physxPlaneShape.h"
#include "physxManager.h"

TypeHandle PhysxPlaneShape::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PhysxPlaneShape::link
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxPlaneShape::
link(NxShape *shapePtr) {

  ref();
  _ptr = shapePtr->isPlane();
  _ptr->userData = this;
  _error_type = ET_ok;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxPlaneShape::unlink
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxPlaneShape::
unlink() {

  _ptr->userData = NULL;
  _error_type = ET_released;
  unref();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxPlaneShape::set_plane
//       Access: Published
//  Description: Sets the plane equation. 
//               - normal:  Normal for the plane, in the global
//                          frame. Range: direction vector  
//               - d: Distance coefficient of the plane equation.
//                    Range: (-inf,inf) 
////////////////////////////////////////////////////////////////////
void PhysxPlaneShape::
set_plane(const LVector3f &normal, float d) {

  nassertv(_error_type == ET_ok);
  _ptr->setPlane(PhysxManager::vec3_to_nxVec3(normal), d);
}

