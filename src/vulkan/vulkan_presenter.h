#pragma once

#include <vector>

#include "../util/log/log.h"

#include "../util/util_error.h"
#include "../util/util_math.h"
#include "../util/util_string.h"

#include "vulkan_loader.h"

namespace dxvk::vk {

  /**
   * \brief Presenter description
   * 
   * Contains the desired properties of
   * the swap chain. This is passed as
   * an input during swap chain creation.
   */
  struct PresenterDesc {
    VkExtent2D          imageExtent;
    uint32_t            imageCount;
    uint32_t            numFormats;
    VkSurfaceFormatKHR  formats[4];
    uint32_t            numPresentModes;
    VkPresentModeKHR    presentModes[4];
    VkFullScreenExclusiveEXT fullScreenExclusive;
  };

  /**
   * \brief Presenter properties
   * 
   * Contains the actual properties
   * of the underlying swap chain.
   */
  struct PresenterInfo {
    VkSurfaceFormatKHR  format;
    VkPresentModeKHR    presentMode;
    VkExtent2D          imageExtent;
    uint32_t            imageCount;
  };

  /**
   * \brief Presenter features
   */
  struct PresenterFeatures {
    bool                fullScreenExclusive : 1;
  };
  
  /**
   * \brief Adapter and queue
   */
  struct PresenterDevice {
    uint32_t            queueFamily = 0;
    VkQueue             queue       = VK_NULL_HANDLE;
    VkPhysicalDevice    adapter     = VK_NULL_HANDLE;
    PresenterFeatures   features    = { };
  };

  /**
   * \brief Swap image and view
   */
  struct PresenterImage {
    VkImage     image = VK_NULL_HANDLE;
    VkImageView view  = VK_NULL_HANDLE;
  };

  /**
   * \brief Presenter semaphores
   * 
   * Pair of semaphores used for acquire and present
   * operations, including the command buffers used
   * in between. Also stores a fence to signal on
   * image acquisition.
   */
  struct PresenterSync {
    VkFence     fence;
    VkSemaphore acquire;
    VkSemaphore present;
  };

  /**
   * \brief Vulkan presenter
   * 
   * Provides abstractions for some of the
   * more complicated aspects of Vulkan's
   * window system integration.
   */
  class Presenter : public RcObject {

  public:

    Presenter(
            HWND            window,
      const Rc<InstanceFn>& vki,
      const Rc<DeviceFn>&   vkd,
            PresenterDevice device,
      const PresenterDesc&  desc);
    
    ~Presenter();

    /**
     * \brief Actual presenter info
     * \returns Swap chain properties
     */
    PresenterInfo info() const;

    /**
     * \breif Retrieves a pair of semaphores
     * 
     * These semaphores are meant to be used
     * for acquire and present operations.
     * \returns Pair of semaphores
     */
    PresenterSync getSyncSemaphores() const;
    
    /**
     * \brief Retrieves image by index
     * 
     * Can be used to create per-image objects.
     * \param [in] index Image index
     * \returns Image handle
     */
    PresenterImage getImage(
            uint32_t        index) const;

    /**
     * \brief Acquires next image
     * 
     * Potentially blocks the calling thread.
     * If this returns an error, the swap chain
     * must be recreated and a new image must
     * be acquired before proceeding.
     * \param [in] signal Semaphore to signal
     * \param [in] fence Fence to signal (optional)
     * \param [out] index Acquired image index
     * \returns Status of the operation
     */
    VkResult acquireNextImage(
            VkSemaphore     signal,
            VkFence         fence,
            uint32_t&       index);
    
    /**
     * \brief Waits for fence to get signaled
     *
     * Helper method that can be used in conjunction
     * with the fence passed to \ref acquireNextImage.
     * \param [in] fence Fence to wait on
     * \returns Status of the operation
     */
    VkResult waitForFence(
            VkFence         fence);
    
    /**
     * \brief Presents current image
     * 
     * Presents the current image. If this returns
     * an error, the swap chain must be recreated,
     * but do not present before acquiring an image.
     * \param [in] wait Semaphore to wait on
     * \returns Status of the operation
     */
    VkResult presentImage(
            VkSemaphore     wait);
    
    /**
     * \brief Changes presenter properties
     * 
     * Recreates the swap chain immediately. Note that
     * no swap chain resources must be in use by the
     * GPU at the time this is called.
     * \param [in] desc Swap chain description
     */
    VkResult recreateSwapChain(
      const PresenterDesc&  desc);

  private:

    Rc<InstanceFn>    m_vki;
    Rc<DeviceFn>      m_vkd;

    PresenterDevice   m_device;
    PresenterInfo     m_info;

    HWND              m_window      = nullptr;
    VkSurfaceKHR      m_surface     = VK_NULL_HANDLE;
    VkSwapchainKHR    m_swapchain   = VK_NULL_HANDLE;

    std::vector<PresenterImage> m_images;
    std::vector<PresenterSync>  m_semaphores;

    uint32_t m_imageIndex = 0;
    uint32_t m_frameIndex = 0;

    VkResult getSupportedFormats(
            std::vector<VkSurfaceFormatKHR>& formats,
      const PresenterDesc&            desc);
    
    VkResult getSupportedPresentModes(
            std::vector<VkPresentModeKHR>& modes,
      const PresenterDesc&            desc);
    
    VkResult getSwapImages(
            std::vector<VkImage>&     images);
    
    VkSurfaceFormatKHR pickFormat(
            uint32_t                  numSupported,
      const VkSurfaceFormatKHR*       pSupported,
            uint32_t                  numDesired,
      const VkSurfaceFormatKHR*       pDesired);

    VkPresentModeKHR pickPresentMode(
            uint32_t                  numSupported,
      const VkPresentModeKHR*         pSupported,
            uint32_t                  numDesired,
      const VkPresentModeKHR*         pDesired);

    VkExtent2D pickImageExtent(
      const VkSurfaceCapabilitiesKHR& caps,
            VkExtent2D                desired);

    uint32_t pickImageCount(
      const VkSurfaceCapabilitiesKHR& caps,
            VkPresentModeKHR          presentMode,
            uint32_t                  desired);

    VkResult createSurface();

    void destroySwapchain();

    void destroySurface();

  };

}
