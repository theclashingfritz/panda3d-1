// Filename: physxActor.h
// Created by:  enn0x (145Sep09)
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

#ifndef PHYSXACTOR_H
#define PHYSXACTOR_H

#include "pandabase.h"
#include "physxObject.h"

#include "nodePath.h"
#include "lmatrix.h"
#include "lvector3.h"
#include "lpoint3.h"
#include "lquaternion.h"

#include "NoMinMax.h"
#include "NxPhysics.h"

class PhysScene;

////////////////////////////////////////////////////////////////////
//       Class : PhysxActor
// Description : Actors are the main simulation objects. Actors
//               are owned by a scene (PhysxScene).
//
//               An actor may optionally encapsulate a dynamic rigid
//               body by setting the body member of the actor's
//               descriptor when it is created. Otherwise the actor
//               will be static (fixed in the world).
//
//               Instances of PhysxActor are created by calling
//               PhysxScene::create_actor() and destroyed by calling
//               PhysxActor::release().
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxActor : public PhysxObject {

PUBLISHED:
  INLINE PhysxActor();
  INLINE ~PhysxActor();

  void set_name( const char *name );
  const char *get_name() const;

////////////////////////////////////////////////////////////////////
PUBLISHED:
  void release();

public:
  INLINE NxActor *ptr() const { return _ptr; };

  void link(NxActor *ptr);
  void unlink();

private:
  NxActor *_ptr;

////////////////////////////////////////////////////////////////////
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PhysxObject::init_type();
    register_type(_type_handle, "PhysxActor", 
                  PhysxObject::get_class_type());
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

#include "physxActor.I"

#endif // PHYSXACTOR_H
