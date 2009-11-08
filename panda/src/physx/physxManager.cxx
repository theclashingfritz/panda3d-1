// Filename: physxManager.cxx
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

#include "physxManager.h"
#include "physxScene.h"
#include "physxSceneDesc.h"

PhysxManager *PhysxManager::_global_ptr;
PhysxManager::PhysxOutputStream PhysxManager::_outputStream;

////////////////////////////////////////////////////////////////////
//     Function: PhysxManager::Constructor
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
PhysxManager::
PhysxManager() {

  // Create PhysX SDK
  NxSDKCreateError error;
  NxPhysicsSDKDesc desc = NxPhysicsSDKDesc();

  _sdk = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION, NULL, &_outputStream, desc, &error);

  if (error == NXCE_NO_ERROR) {
    physx_cat.info() << "PhysX subsystem initialized. Number of PPUs="
                     << _sdk->getNbPPUs() << endl;
  }
  else {
    physx_cat.error() << "Error when setting up the PhysX subsystem: "
                      << get_sdk_error_string(error) << endl;
  }

  nassertv_always(error == NXCE_NO_ERROR);
  nassertv_always(_sdk);
  
  // Set some default parameters
  _sdk->setParameter(NX_VISUALIZATION_SCALE, 0.0f);
  _sdk->setParameter(NX_VISUALIZE_COLLISION_SHAPES, true);
  _sdk->setParameter(NX_VISUALIZE_ACTOR_AXES, true);
  _sdk->setParameter(NX_VISUALIZE_BODY_LIN_VELOCITY, true);
  _sdk->setParameter(NX_VISUALIZE_COLLISION_AABBS, false);
  _sdk->setParameter(NX_VISUALIZE_COLLISION_VNORMALS, false);
  _sdk->setParameter(NX_VISUALIZE_COLLISION_FNORMALS, false);
  _sdk->setParameter(NX_VISUALIZE_FORCE_FIELDS, false);

  // Connect VDR
  if(physx_want_vrd) {
    physx_cat.info() << "Connecting to visual remote debugger at ("
                     << physx_vrd_host << ":"
                     << physx_vrd_port << ")" << endl;

    NxRemoteDebugger *debugger = _sdk->getFoundationSDK().getRemoteDebugger();

    debugger->connect(physx_vrd_host.c_str(),
                      physx_vrd_port);

    if (!debugger->isConnected()) {
      physx_cat.warning() << "Could not connect to visual remot debugger!" << endl;
    }
  }

}

////////////////////////////////////////////////////////////////////
//     Function: PhysxManager::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PhysxManager::
~PhysxManager() {

  // Disconnect VRD
  if(physx_want_vrd) {
    NxRemoteDebugger *debugger = _sdk->getFoundationSDK().getRemoteDebugger();
    if (!debugger->isConnected()) {
      debugger->disconnect();
    }
  }

  // Release PhysX SDK
  NxReleasePhysicsSDK(_sdk);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxManager::get_global_ptr
//       Access: Published
//  Description: Returns a pointer to the global PhysxManager
//               object.
////////////////////////////////////////////////////////////////////
PhysxManager *PhysxManager::
get_global_ptr() {

  if (_global_ptr == (PhysxManager *)NULL) {
    _global_ptr = new PhysxManager;
  }

  return _global_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxManager::get_num_scenes
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
unsigned int PhysxManager::
get_num_scenes() const {

  return _sdk->getNbScenes();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxManager::create_scene
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PT(PhysxScene) PhysxManager::
create_scene(PhysxSceneDesc &desc) {

  nassertr(desc.is_valid(),NULL);

  //_desc.timeStepMethod = NX_TIMESTEP_FIXED;
  //_desc.maxTimestep = 1.0f / 240.0f;
  //_desc.maxIter = 8;

  desc._desc.flags |= NX_SF_ENABLE_ACTIVETRANSFORMS;
  desc._desc.flags |= NX_SF_SIMULATE_SEPARATE_THREAD;

  PT(PhysxScene) scene = new PhysxScene();
  nassertr(scene, NULL);

  NxScene *scenePtr = _sdk->createScene(desc._desc);
  nassertr(scenePtr, NULL);

  scene->link(scenePtr);

  return scene;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxManager::get_scene
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PT(PhysxScene) PhysxManager::
get_scene(unsigned int idx) const {

  nassertr_always(idx < _sdk->getNbScenes(), NULL);

  NxScene *scenePtr = _sdk->getScene(idx);
  PhysxScene *scene = (PhysxScene *)(scenePtr->userData);

  return scene;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxManager::get_num_height_fields
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
unsigned int PhysxManager::
get_num_height_fields() {

  return _sdk->getNbHeightFields();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxManager::create_height_field
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PT(PhysxHeightField) PhysxManager::
create_height_field(PhysxHeightFieldDesc &desc) {

  nassertr(desc.is_valid(),NULL);

  PT(PhysxHeightField) hf = new PhysxHeightField();
  nassertr(hf, NULL);

  NxHeightField *hfPtr = _sdk->createHeightField(desc._desc);
  nassertr(hfPtr, NULL);

  hf->link(hfPtr);

  return hf;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxManager::get_height_field
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PT(PhysxHeightField) PhysxManager::
get_height_field(unsigned int idx) {

  nassertr_always(idx < _sdk->getNbHeightFields(), NULL);

  return (PhysxHeightField *)_heightfields[idx];
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxManager::is_hardware_available
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
bool PhysxManager::
is_hardware_available() {

  return _sdk->getHWVersion() != NX_HW_VERSION_NONE;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxManager::set_parameter
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void PhysxManager::
set_parameter(PhysxParameter param, float value) {

  _sdk->setParameter((NxParameter)param, value);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxManager::get_parameter
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
float PhysxManager::
get_parameter(PhysxParameter param) {

  return _sdk->getParameter((NxParameter)param);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxManager::get_sdk_error_string
//       Access: Private
//  Description: Returns the NxSDKCreateError enum as string.
////////////////////////////////////////////////////////////////////
const char *PhysxManager::
get_sdk_error_string(const NxSDKCreateError &error) {

  switch (error) {
  case NXCE_NO_ERROR:           return "NXCE_NO_ERROR"; break;
  case NXCE_PHYSX_NOT_FOUND:    return "NXCE_PHYSX_NOT_FOUND"; break;
  case NXCE_WRONG_VERSION:      return "NXCE_WRONG_VERSION"; break;
  case NXCE_DESCRIPTOR_INVALID: return "NXCE_DESCRIPTOR_INVALID"; break;
  case NXCE_CONNECTION_ERROR:   return "NXCE_CONNECTION_ERROR"; break;
  case NXCE_RESET_ERROR:        return "NXCE_RESET_ERROR"; break;
  case NXCE_IN_USE_ERROR:       return "NXCE_IN_USE_ERROR"; break;
  case NXCE_BUNDLE_ERROR:       return "NXCE_BUNDLE_ERROR"; break;
  default:                      return "Unknown error"; break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxOutputStream::reportError
//       Access: Private
//  Description: Reports an error code from the PhysX SDK.
////////////////////////////////////////////////////////////////////
void PhysxManager::PhysxOutputStream::
reportError(NxErrorCode code, const char *message, const char *file, int line) {

  physx_cat.error() << get_error_code_string(code) << ": " 
                    << message << endl;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxOutputStream::get_error_code_string
//       Access: Private
//  Description: Returns the NxSDKCreateError enum as string.
////////////////////////////////////////////////////////////////////
const char *PhysxManager::PhysxOutputStream::
get_error_code_string(NxErrorCode code) {
  
  switch (code) {
  case NXE_NO_ERROR:           return "NO_ERROR"; break;
  case NXE_INVALID_PARAMETER:  return "INVALID_PARAMETER"; break;
  case NXE_INVALID_OPERATION:  return "INVALID_OPERATION"; break;
  case NXE_OUT_OF_MEMORY:      return "OUT_OF_MEMORY"; break;
  case NXE_INTERNAL_ERROR:     return "INTERNAL_ERROR"; break;
  case NXE_ASSERTION:          return "ASSERTION"; break;
  case NXE_DB_INFO:            return "DB_INFO"; break;
  case NXE_DB_WARNING:         return "DB_WARNING"; break;
  case NXE_DB_PRINT:           return "DB_PRINT"; break;
  default:                     return ""; break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxOutputStream::reportAssertViolation
//       Access: Private
//  Description: Reports an assertion violation from the PhysX SDK.
////////////////////////////////////////////////////////////////////
NxAssertResponse PhysxManager::PhysxOutputStream::
reportAssertViolation(const char *message, const char *file, int line) {

  physx_cat.error() << "AssertViolation: " << message << endl;

  return NX_AR_CONTINUE;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxOutputStream::print
//       Access: Private
//  Description: Prints some debug text from the PhysX SDK.
////////////////////////////////////////////////////////////////////
void PhysxManager::PhysxOutputStream::
print(const char *message) {

  nout << message;
}

