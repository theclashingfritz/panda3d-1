// Filename: physxJointDesc.h
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

#ifndef PHYSXJOINTDESC_H
#define PHYSXJOINTDESC_H

#include "pandabase.h"
#include "typedReferenceCount.h"
#include "lpoint3.h"
#include "lvector3.h"

#include "physxEnums.h"

#include "NoMinMax.h"
#include "NxPhysics.h"

class PhysxActor;

////////////////////////////////////////////////////////////////////
//       Class : PhysxJointDesc
// Description : Abstract base class for joint descriptors.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxJointDesc : public TypedReferenceCount, public PhysxEnums {

PUBLISHED:
  INLINE PhysxJointDesc();
  INLINE ~PhysxJointDesc();

  virtual void set_to_default() = 0;
  virtual bool is_valid() const = 0;

  void set_name(const char *name);
  void set_joint_flag(PhysxJointFlag flag, bool value);
  void set_max_force(float force);
  void set_max_torque(float torque);
  void set_solver_extrapolation_factor(float factor);
  void set_global_axis(const LVector3f &axis);
  void set_global_anchor(const LPoint3f &anchor);
  void set_local_normal(unsigned int idx, const LVector3f &normal);
  void set_local_axis(unsigned int idx, const LVector3f &axis);
  void set_local_anchor(unsigned int idx, const LPoint3f &anchor);
  void set_actor(unsigned int idx, const PhysxActor &actor);

  const char *get_name() const;
  bool get_joint_flag(PhysxJointFlag flag) const;
  float get_max_force() const;
  float get_max_torque() const;
  float get_solver_extrapolation_factor() const;
  LVector3f get_local_normal(unsigned int idx) const;
  LVector3f get_local_axis(unsigned int idx) const;
  LPoint3f get_local_anchor(unsigned int idx) const;
  PT(PhysxActor) get_actor(unsigned int idx) const;

public:
  virtual NxJointDesc *ptr() const = 0;

////////////////////////////////////////////////////////////////////
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "PhysxJointDesc", 
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
    init_type();
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};

#include "physxJointDesc.I"

#endif // PHYSXJOINTDESC_H
