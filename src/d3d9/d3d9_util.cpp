#include "d3d9_util.h"

namespace dxvk {

  HRESULT DecodeMultiSampleType(
        D3DMULTISAMPLE_TYPE       MultiSample,
        VkSampleCountFlagBits*    pCount) {
    VkSampleCountFlagBits flag;

    switch (MultiSample) {
    case D3DMULTISAMPLE_NONE:
    case D3DMULTISAMPLE_NONMASKABLE: flag = VK_SAMPLE_COUNT_1_BIT;  break;
    case D3DMULTISAMPLE_2_SAMPLES: flag = VK_SAMPLE_COUNT_2_BIT;  break;
    case D3DMULTISAMPLE_4_SAMPLES: flag = VK_SAMPLE_COUNT_4_BIT;  break;
    case D3DMULTISAMPLE_8_SAMPLES: flag = VK_SAMPLE_COUNT_8_BIT;  break;
    case D3DMULTISAMPLE_16_SAMPLES: flag = VK_SAMPLE_COUNT_16_BIT; break;
    default: return D3DERR_INVALIDCALL;
    }

    if (pCount != nullptr)
      *pCount = flag;

    return D3D_OK;
  }

  bool    ResourceBindable(
      DWORD                     Usage,
      D3DPOOL                   Pool) {
    return true;
  }

  VkFormat GetPackedDepthStencilFormat(D3D9Format Format) {
    switch (Format) {
    case D3D9Format::D15S1:
      return VK_FORMAT_D16_UNORM_S8_UINT; // This should never happen!

    case D3D9Format::D16:
    case D3D9Format::D16_LOCKABLE:
    case D3D9Format::DF16:
      return VK_FORMAT_D16_UNORM;

    case D3D9Format::D24X8:
    case D3D9Format::DF24:
      return VK_FORMAT_X8_D24_UNORM_PACK32;

    case D3D9Format::D24X4S4:
    case D3D9Format::D24FS8:
    case D3D9Format::D24S8:
    case D3D9Format::INTZ:
      return VK_FORMAT_D24_UNORM_S8_UINT;

    case D3D9Format::D32:
    case D3D9Format::D32_LOCKABLE:
    case D3D9Format::D32F_LOCKABLE:
      return VK_FORMAT_D32_SFLOAT;

    case D3D9Format::S8_LOCKABLE:
      return VK_FORMAT_S8_UINT;

    default:
      return VK_FORMAT_UNDEFINED;
    }
  }

  VkFormatFeatureFlags GetImageFormatFeatures(DWORD Usage) {
    VkFormatFeatureFlags features = VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;

    if (Usage & D3DUSAGE_DEPTHSTENCIL)
      features |= VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

    if (Usage & D3DUSAGE_RENDERTARGET)
      features |= VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT;

    return features;
  }

  VkImageUsageFlags GetImageUsageFlags(DWORD Usage) {
    VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    if (Usage & D3DUSAGE_DEPTHSTENCIL)
      usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    if (Usage & D3DUSAGE_RENDERTARGET)
      usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    return usage;
  }

  VkMemoryPropertyFlags GetMemoryFlagsForUsage(
          DWORD                   Usage) {
    VkMemoryPropertyFlags memoryFlags = 0;

    if (Usage & D3DUSAGE_DYNAMIC) {
      memoryFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                  |  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    }
    else {
      memoryFlags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    }

    return memoryFlags;
  }

  uint32_t VertexCount(D3DPRIMITIVETYPE type, UINT count) {
    switch (type) {
      default:
      case D3DPT_TRIANGLELIST: return count * 3;
      case D3DPT_POINTLIST: return count;
      case D3DPT_LINELIST: return count * 2;
      case D3DPT_LINESTRIP: return count + 1;
      case D3DPT_TRIANGLESTRIP: return count + 2;
      case D3DPT_TRIANGLEFAN: return count + 2;
    }
  }

  DxvkInputAssemblyState InputAssemblyState(D3DPRIMITIVETYPE type) {
    switch (type) {
      default:
      case D3DPT_TRIANGLELIST:
        return { VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE, 0 };

      case D3DPT_POINTLIST:
        return { VK_PRIMITIVE_TOPOLOGY_POINT_LIST, VK_FALSE, 0 };

      case D3DPT_LINELIST:
        return { VK_PRIMITIVE_TOPOLOGY_LINE_LIST, VK_FALSE, 0 };

      case D3DPT_LINESTRIP:
        return { VK_PRIMITIVE_TOPOLOGY_LINE_STRIP, VK_TRUE, 0 };

      case D3DPT_TRIANGLESTRIP:
        return { VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, VK_TRUE, 0 };

      case D3DPT_TRIANGLEFAN:
        return { VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN, VK_TRUE, 0 };
    }
  }

  VkBlendFactor DecodeBlendFactor(D3DBLEND BlendFactor, bool IsAlpha) {
    switch (BlendFactor) {
      case D3DBLEND_ZERO:            return VK_BLEND_FACTOR_ZERO;
      default:
      case D3DBLEND_ONE:             return VK_BLEND_FACTOR_ONE;
      case D3DBLEND_SRCCOLOR:        return VK_BLEND_FACTOR_SRC_COLOR;
      case D3DBLEND_INVSRCCOLOR:     return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
      case D3DBLEND_SRCALPHA:        return VK_BLEND_FACTOR_SRC_ALPHA;
      case D3DBLEND_INVSRCALPHA:     return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
      case D3DBLEND_DESTALPHA:       return VK_BLEND_FACTOR_DST_ALPHA;
      case D3DBLEND_INVDESTALPHA:    return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
      case D3DBLEND_DESTCOLOR:       return VK_BLEND_FACTOR_DST_COLOR;
      case D3DBLEND_INVDESTCOLOR:    return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
      case D3DBLEND_SRCALPHASAT:     return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
      case D3DBLEND_BOTHSRCALPHA:    return VK_BLEND_FACTOR_SRC1_ALPHA;
      case D3DBLEND_BOTHINVSRCALPHA: return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
      case D3DBLEND_BLENDFACTOR:     return IsAlpha ? VK_BLEND_FACTOR_CONSTANT_ALPHA : VK_BLEND_FACTOR_CONSTANT_COLOR;
      case D3DBLEND_INVBLENDFACTOR:  return IsAlpha ? VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA : VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
      case D3DBLEND_SRCCOLOR2:       return VK_BLEND_FACTOR_SRC1_COLOR;
      case D3DBLEND_INVSRCCOLOR2:    return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
    }
  }

  VkBlendOp DecodeBlendOp(D3DBLENDOP BlendOp) {
    switch (BlendOp) {
      default:
      case D3DBLENDOP_ADD:          return VK_BLEND_OP_ADD;
      case D3DBLENDOP_SUBTRACT:     return VK_BLEND_OP_SUBTRACT;
      case D3DBLENDOP_REVSUBTRACT:  return VK_BLEND_OP_REVERSE_SUBTRACT;
      case D3DBLENDOP_MIN:          return VK_BLEND_OP_MIN;
      case D3DBLENDOP_MAX:          return VK_BLEND_OP_MAX;
    }
  }

}