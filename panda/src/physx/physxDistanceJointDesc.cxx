// Filename: physxDistanceJointDesc.cxx
// Created by:  enn0x (28Sep09)
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

#include "physxDistanceJointDesc.h"
//#include "physxSpringDesc.h"

TypeHandle PhysxDistanceJointDesc::_type_handle;

/*
////////////////////////////////////////////////////////////////////
//     Function: PhysxDistanceJointDesc::set_max_distance
//       Access: Published
//  Description: Sets the maximum rest length of the rope or rod
//               between the two anchor points.
////////////////////////////////////////////////////////////////////
void PhysxDistanceJointDesc::
set_max_distance(float distance) {

  _desc.maxDistance = distance;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxDistanceJointDesc::set_min_distance
//       Access: Published
//  Description: Sets the minimum rest length of the rope or rod
//               between the two anchor points
////////////////////////////////////////////////////////////////////
void PhysxDistanceJointDesc::
set_min_distance(float distance) {

  _desc.minDistance = distance;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxDistanceJointDesc::set_spring
//       Access: Published
//  Description: Makes the joint springy. The spring.targetValue
//               is not used. 
////////////////////////////////////////////////////////////////////
void PhysxDistanceJointDesc::
set_spring(const PhysxSpringDesc &spring) {

  _desc.spring = spring.desc();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxDistanceJointDesc::set_flag
//       Access: Published
//  Description: Sets or clears a single DistanceJointFlag flag.
////////////////////////////////////////////////////////////////////
void PhysxDistanceJointDesc::
set_flag(PhysxDistanceJointFlag flag, bool value) {

  if (value == true) {
    _desc.flags |= flag;
  }
  else {
    _desc.flags &= ~(flag);
  }
}
*/
