// Filename: omitReason.h
// Created by:  drose (30Nov00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef OMITREASON_H
#define OMITREASON_H

#include "pandatoolbase.h"

////////////////////////////////////////////////////////////////////
//        Enum : OmitReason
// Description : This enumerates the reasons why a texture may not
//               have been placed in a palette image.
////////////////////////////////////////////////////////////////////
enum OmitReason {
  OR_none,
  // Not omitted: the texture appears on a palette image.

  OR_working,
  // Still working on placing it.

  OR_omitted,
  // Explicitly omitted by the user via "omit" in .txa file.

  OR_size,
  // Too big to fit on a single palette image.

  OR_solitary,
  // It should be placed, but it's the only one on the palette image
  // so far, so there's no point.

  OR_coverage,
  // The texture repeats.  Specifically, the UV's for the texture
  // exceed the maximum rectangle allowed by coverage_threshold.

  OR_unknown,
  // The texture file cannot be read, so its size can't be determined.

  OR_unused,
  // The texture is no longer used by any of the egg files that
  // formerly referenced it.
};

ostream &operator << (ostream &out, OmitReason omit);

#endif

