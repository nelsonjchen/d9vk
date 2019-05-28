#pragma once

#include "d3d9_subresource.h"

#include "d3d9_common_texture.h"

#include "../util/util_gdi.h"

namespace dxvk {

  using D3D9GDIDesc = D3DKMT_DESTROYDCFROMMEMORY;

  using D3D9SurfaceBase = D3D9Subresource<IDirect3DSurface9>;
  class D3D9Surface final : public D3D9SurfaceBase {

  public:

    D3D9Surface(
            D3D9DeviceEx*             pDevice,
      const D3D9_COMMON_TEXTURE_DESC* pDesc);

    D3D9Surface(
            D3D9DeviceEx*             pDevice,
            D3D9CommonTexture*        pTexture,
            UINT                      Face,
            UINT                      MipLevel,
            IDirect3DBaseTexture9*    pContainer);

    void AddRefPrivate();

    void ReleasePrivate();

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject);

    D3DRESOURCETYPE STDMETHODCALLTYPE GetType() final;

    HRESULT STDMETHODCALLTYPE GetDesc(D3DSURFACE_DESC *pDesc) final;

    HRESULT STDMETHODCALLTYPE LockRect(D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags) final;

    HRESULT STDMETHODCALLTYPE UnlockRect() final;

    HRESULT STDMETHODCALLTYPE GetDC(HDC *phDC) final;

    HRESULT STDMETHODCALLTYPE ReleaseDC(HDC hDC) final;

  private:

    D3D9GDIDesc m_dcDesc;

  };
}