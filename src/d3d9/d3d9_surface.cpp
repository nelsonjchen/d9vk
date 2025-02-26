#include "d3d9_surface.h"
#include "d3d9_texture.h"

namespace dxvk {

  D3D9Surface::D3D9Surface(
          D3D9DeviceEx*             pDevice,
    const D3D9_COMMON_TEXTURE_DESC* pDesc,
          D3D9_VK_FORMAT_MAPPING    Mapping)
    : D3D9SurfaceBase(
        pDevice,
        new D3D9CommonTexture( pDevice, pDesc, D3DRTYPE_TEXTURE, Mapping ),
        0, 0,
        nullptr) { }

  D3D9Surface::D3D9Surface(
          D3D9DeviceEx*             pDevice,
          D3D9CommonTexture*        pTexture,
          UINT                      Face,
          UINT                      MipLevel,
          IDirect3DBaseTexture9*    pContainer)
    : D3D9SurfaceBase(
        pDevice,
        pTexture,
        Face, MipLevel,
        pContainer) { }

  void D3D9Surface::AddRefPrivate() {
    IDirect3DBaseTexture9* pContainer = this->m_container;

    if (pContainer != nullptr) {
      D3DRESOURCETYPE type = pContainer->GetType();
      if (type == D3DRTYPE_TEXTURE)
        reinterpret_cast<D3D9Texture2D*>  (pContainer)->AddRefPrivate();
      else //if (type == D3DRTYPE_CUBETEXTURE)
        reinterpret_cast<D3D9TextureCube*>(pContainer)->AddRefPrivate();

      return;
    }

    D3D9SurfaceBase::AddRefPrivate();
  }

  void D3D9Surface::ReleasePrivate() {
    IDirect3DBaseTexture9* pContainer = this->m_container;

    if (pContainer != nullptr) {
      D3DRESOURCETYPE type = pContainer->GetType();
      if (type == D3DRTYPE_TEXTURE)
        reinterpret_cast<D3D9Texture2D*>  (pContainer)->ReleasePrivate();
      else //if (type == D3DRTYPE_CUBETEXTURE)
        reinterpret_cast<D3D9TextureCube*>(pContainer)->ReleasePrivate();

      return;
    }

    D3D9SurfaceBase::ReleasePrivate();
  }

  HRESULT STDMETHODCALLTYPE D3D9Surface::QueryInterface(REFIID riid, void** ppvObject) {
    if (ppvObject == nullptr)
      return E_POINTER;

    *ppvObject = nullptr;

    if (riid == __uuidof(IUnknown)
     || riid == __uuidof(IDirect3DResource9)
     || riid == __uuidof(IDirect3DSurface9)) {
      *ppvObject = ref(this);
      return S_OK;
    }

    Logger::warn("D3D9Surface::QueryInterface: Unknown interface query");
    Logger::warn(str::format(riid));
    return E_NOINTERFACE;
  }

  D3DRESOURCETYPE STDMETHODCALLTYPE D3D9Surface::GetType() {
    return D3DRTYPE_SURFACE;
  }

  HRESULT STDMETHODCALLTYPE D3D9Surface::GetDesc(D3DSURFACE_DESC *pDesc) {
    if (pDesc == nullptr)
      return D3DERR_INVALIDCALL;

    auto& desc = *(m_texture->Desc());

    pDesc->Format             = static_cast<D3DFORMAT>(desc.Format);
    pDesc->Type               = D3DRTYPE_SURFACE;
    pDesc->Usage              = desc.Usage;
    pDesc->Pool               = desc.Pool;
    
    pDesc->MultiSampleType    = desc.MultiSample;
    pDesc->MultiSampleQuality = desc.MultisampleQuality;
    pDesc->Width              = std::max(1u, desc.Width >> m_mipLevel);
    pDesc->Height             = std::max(1u, desc.Height >> m_mipLevel);

    return D3D_OK;
  }

  HRESULT STDMETHODCALLTYPE D3D9Surface::LockRect(D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags) {
    D3DBOX box;
    if (pRect != nullptr) {
      box.Left   = pRect->left;
      box.Right  = pRect->right;
      box.Top    = pRect->top;
      box.Bottom = pRect->bottom;
      box.Front  = 0;
      box.Back   = 1;
    }

    D3DLOCKED_BOX lockedBox;

    HRESULT hr = m_parent->LockImage(
      m_texture,
      m_face, m_mipLevel,
      &lockedBox,
      pRect != nullptr ? &box : nullptr,
      Flags);

    pLockedRect->pBits = lockedBox.pBits;
    pLockedRect->Pitch = lockedBox.RowPitch;

    return hr;
  }

  HRESULT STDMETHODCALLTYPE D3D9Surface::UnlockRect() {
    return m_parent->UnlockImage(
      m_texture,
      m_face, m_mipLevel);
  }

  HRESULT STDMETHODCALLTYPE D3D9Surface::GetDC(HDC *phDC) {
    if (phDC == nullptr)
      return D3DERR_INVALIDCALL;

    const D3D9_COMMON_TEXTURE_DESC& desc = *m_texture->Desc();

    D3DLOCKED_RECT lockedRect;
    HRESULT hr = LockRect(&lockedRect, nullptr, 0);
    if (FAILED(hr))
      return hr;

    D3DKMT_CREATEDCFROMMEMORY createInfo;
    // In...
    createInfo.pMemory     = lockedRect.pBits;
    createInfo.Format      = static_cast<D3DFORMAT>(desc.Format);
    createInfo.Width       = desc.Width;
    createInfo.Height      = desc.Height;
    createInfo.Pitch       = lockedRect.Pitch;
    createInfo.hDeviceDc   = CreateCompatibleDC(NULL);
    createInfo.pColorTable = nullptr;

    // Out...
    createInfo.hBitmap     = nullptr;
    createInfo.hDc         = nullptr;

    D3DKMTCreateDCFromMemory(&createInfo);
    DeleteDC(createInfo.hDeviceDc);

    // These should now be set...
    m_dcDesc.hDC     = createInfo.hDc;
    m_dcDesc.hBitmap = createInfo.hBitmap;

    *phDC = m_dcDesc.hDC;
    return D3D_OK;
  }

  HRESULT STDMETHODCALLTYPE D3D9Surface::ReleaseDC(HDC hDC) {
    if (m_dcDesc.hDC == nullptr || m_dcDesc.hDC != hDC)
      return D3DERR_INVALIDCALL;

    D3DKMTDestroyDCFromMemory(&m_dcDesc);

    HRESULT hr = UnlockRect();
    if (FAILED(hr))
      return hr;

    return D3D_OK;
  }

}
