// Filename: physxManager.h
// Created by:  enn0x (01Sep09)
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

#ifndef PHYSXMANAGER_H
#define PHYSXMANAGER_H

#include "pandabase.h"
#include "pointerTo.h"
#include "luse.h"

#include "NoMinMax.h"
#include "NxPhysics.h"
#include "NxExtended.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxManager
// Description : The central interface to the PhysX subsystem.
//               Used e. g. for setting/retrieving global parameters
//               or for creating scenes.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxManager {

PUBLISHED:
  PhysxManager();
  ~PhysxManager();

  bool is_hardware_available();

public:
  INLINE static NxVec3 vec3_to_nxVec3(const LVector3f &v);
  INLINE static LVector3f nxVec3_to_vec3(const NxVec3 &v);
  INLINE static NxExtendedVec3 vec3_to_nxExtVec3(const LVector3f &v);
  INLINE static LVector3f nxExtVec3_to_vec3(const NxExtendedVec3 &v);
  INLINE static NxVec3 point3_to_nxVec3(const LPoint3f &p);
  INLINE static LPoint3f nxVec3_to_point3(const NxVec3 &p);
  INLINE static NxExtendedVec3 point3_to_nxExtVec3(const LPoint3f &p);
  INLINE static LPoint3f nxExtVec3_to_point3(const NxExtendedVec3 &p);
  INLINE static NxQuat quat_to_nxQuat(const LQuaternionf &q);
  INLINE static LQuaternionf nxQuat_to_quat(const NxQuat &q);
  INLINE static NxMat34 mat4_to_nxMat34(const LMatrix4f &m);
  INLINE static LMatrix4f nxMat34_to_mat4(const NxMat34 &m);
  INLINE static NxMat33 mat3_to_nxMat33(const LMatrix3f &m);
  INLINE static LMatrix3f nxMat33_to_mat3(const NxMat33 &m);

private:
  NxPhysicsSDK *_sdk;

  class PhysxOutputStream : public NxUserOutputStream {
    void reportError(NxErrorCode code, const char *message, const char *file, int line);
    NxAssertResponse reportAssertViolation(const char *message, const char *file, int line);
    void print(const char *message);
    const char *get_error_code_string(NxErrorCode code);
  };
  static PhysxOutputStream _outputStream;

  static const char *get_sdk_error_string(const NxSDKCreateError &error);
};

#include "physxManager.I"

#endif // PHYSXMANAGER_H