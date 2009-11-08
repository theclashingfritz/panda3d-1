// Filename: physxObjectCollection.cxx
// Created by:  enn0x (08Nov09)
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

#include "physxObjectCollection.h"

////////////////////////////////////////////////////////////////////
//     Function: PhysxObjectCollection<T>::size
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template <class T>
unsigned int PhysxObjectCollection<T>::
size() const {

  return _objects.size();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxObjectCollection<T>::add
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template <class T>
void PhysxObjectCollection<T>::
add(PT(T) object) {

  _objects.push_back(object);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxObjectCollection<T>::remove
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template <class T>
void PhysxObjectCollection<T>::
remove(PT(T) object) {

  pvector<PT(T)>::iterator it;

  it = find(_objects.begin(), _objects.end(), object);
  if (it != _objects.end()) {
    _objects.erase(it);
  }
  else
  {
    physx_cat.warning() << "object not found in collection" << endl;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxObjectCollection<T>::get
//       Access: Public
//  Description: Returns the n-th PhysObject in the collection.
//               The operator [] is maybe a more convenient way to
//               access objects from the collection.
////////////////////////////////////////////////////////////////////
template <class T>
T *PhysxObjectCollection<T>::
get(unsigned int index) const {

  nassertr(index < _objects.size(), NULL);
  return _objects[index];
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxObjectCollection<T>::operator []
//       Access: Public
//  Description: Returns the n-th PhysObject in the collection.
//               This is the same as the get() method.
////////////////////////////////////////////////////////////////////
template <class T>
T *PhysxObjectCollection<T>::
operator [] (unsigned int index) const {

  nassertr(index < _objects.size(), NULL);
  return _objects[index];
}

