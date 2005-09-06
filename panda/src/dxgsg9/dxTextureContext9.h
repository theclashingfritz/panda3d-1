// Filename: dxTextureContext8.h
// Created by:   masad (02Jan04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef DXTEXTURECONTEXT9_H
#define DXTEXTURECONTEXT9_H

#include "dxgsg9base.h"
#include "texture.h"
#include "textureContext.h"

//#define DO_CUSTOM_CONVERSIONS

////////////////////////////////////////////////////////////////////
//   Class : DXTextureContext9
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDADX DXTextureContext9 : public TextureContext {
  friend class DXGraphicsStateGuardian;
  friend class wdxGraphicsWindow;

public:
  DXTextureContext9(Texture *tex);
  ~DXTextureContext9();

  IDirect3DTexture9  *_pD3DTexture9;
  Texture *_tex;            // ptr to parent, primarily for access to namestr
  IDirect3DTexture9 *CreateTexture(DXScreenData &scrn);

  D3DFORMAT _PixBufD3DFmt;    // the 'D3DFORMAT' the Panda TextureBuffer fmt corresponds to

  bool _bHasMipMaps;

#ifdef DO_CUSTOM_CONVERSIONS
  DWORD _PixBufConversionType;  // enum ConversionType
#endif

  // must be public since called from global callback fns
  void DeleteTexture();
  HRESULT FillDDSurfTexturePixels();

protected:
    unsigned int get_bits_per_pixel(Texture::Format format, int *alphbits);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TextureContext::init_type();
    register_type(_type_handle, "DXTextureContext9",
          TextureContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

extern HRESULT ConvertD3DSurftoPixBuf(RECT &SrcRect,IDirect3DSurface9 *pD3DSurf9,Texture *pixbuf);

#endif

