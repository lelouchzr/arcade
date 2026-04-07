#pragma once
#include "../common/RasterCanvas.hpp"
#include "../common/SdlHost.hpp"
#include <vulkan/vulkan.h>

namespace arcade::bonus {
class Vulkan final : public IGraphics {
  public:
    ~Vulkan() override;
    void open() override;
    void close() noexcept override;
    std::vector<Input> poll() override;
    void draw(const Frame &frame) override;

  private:
    SdlHost host_;
    RasterCanvas canvas_;
    VkInstance instance_ = VK_NULL_HANDLE;
    VkSurfaceKHR surface_ = VK_NULL_HANDLE;
    VkPhysicalDevice physical_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;
    VkQueue queue_ = VK_NULL_HANDLE;
    std::uint32_t family_ = 0;
    VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;
    VkFormat format_ = VK_FORMAT_UNDEFINED;
    VkExtent2D extent_{};
    std::vector<VkImage> images_;
    VkCommandPool pool_ = VK_NULL_HANDLE;
    VkCommandBuffer command_ = VK_NULL_HANDLE;
    VkSemaphore acquired_ = VK_NULL_HANDLE, finished_ = VK_NULL_HANDLE;
    VkBuffer staging_ = VK_NULL_HANDLE;
    VkDeviceMemory memory_ = VK_NULL_HANDLE;
    void selectDevice();
    bool recreate();
    void releaseSwapchain() noexcept;
    void record(std::uint32_t image);
    static void check(VkResult result, const char *operation);
};
} // namespace arcade::bonus
