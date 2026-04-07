#include "Vulkan.hpp"
#include <SDL_vulkan.h>
#include <algorithm>
#include <cstring>
#include <limits>
#include <stdexcept>

namespace arcade::bonus {
void Vulkan::check(VkResult value, const char *operation) {
    if (value != VK_SUCCESS)
        throw std::runtime_error(std::string("Vulkan ") + operation + ": " + std::to_string(value));
}

Vulkan::~Vulkan() {
    close();
}

void Vulkan::selectDevice() {
    std::uint32_t count = 0;
    check(vkEnumeratePhysicalDevices(instance_, &count, nullptr), "enumerate devices");
    std::vector<VkPhysicalDevice> devices(count);
    check(vkEnumeratePhysicalDevices(instance_, &count, devices.data()), "enumerate devices");
    for (auto device : devices) {
        std::uint32_t families = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &families, nullptr);
        std::vector<VkQueueFamilyProperties> properties(families);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &families, properties.data());
        for (std::uint32_t i = 0; i < families; ++i) {
            VkBool32 present = VK_FALSE;
            check(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface_, &present),
                  "surface support");
            if (present && (properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
                physical_ = device;
                family_ = i;
                return;
            }
        }
    }
    throw std::runtime_error("Vulkan: no graphics queue capable of presentation");
}

void Vulkan::open() {
    if (device_)
        return;
    try {
        host_.open("ARCADE / 400 - Vulkan", SDL_WINDOW_VULKAN);
        unsigned count = 0;
        if (!SDL_Vulkan_GetInstanceExtensions(host_.window(), &count, nullptr))
            throw std::runtime_error(SDL_GetError());
        std::vector<const char *> extensions(count);
        if (!SDL_Vulkan_GetInstanceExtensions(host_.window(), &count, extensions.data()))
            throw std::runtime_error(SDL_GetError());
        VkApplicationInfo application{};
        application.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        application.pApplicationName = "Arcade";
        application.apiVersion = VK_API_VERSION_1_0;
        VkInstanceCreateInfo instance{};
        instance.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instance.pApplicationInfo = &application;
        instance.enabledExtensionCount = count;
        instance.ppEnabledExtensionNames = extensions.data();
        check(vkCreateInstance(&instance, nullptr, &instance_), "create instance");
        if (!SDL_Vulkan_CreateSurface(host_.window(), instance_, &surface_))
            throw std::runtime_error(SDL_GetError());
        selectDevice();
        const float priority = 1;
        VkDeviceQueueCreateInfo queue{};
        queue.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue.queueFamilyIndex = family_;
        queue.queueCount = 1;
        queue.pQueuePriorities = &priority;
        const char *extension = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
        VkDeviceCreateInfo device{};
        device.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        device.queueCreateInfoCount = 1;
        device.pQueueCreateInfos = &queue;
        device.enabledExtensionCount = 1;
        device.ppEnabledExtensionNames = &extension;
        check(vkCreateDevice(physical_, &device, nullptr, &device_), "create device");
        vkGetDeviceQueue(device_, family_, 0, &queue_);
        VkCommandPoolCreateInfo pool{};
        pool.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pool.queueFamilyIndex = family_;
        pool.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        check(vkCreateCommandPool(device_, &pool, nullptr, &pool_), "create command pool");
        VkCommandBufferAllocateInfo buffer{};
        buffer.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        buffer.commandPool = pool_;
        buffer.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        buffer.commandBufferCount = 1;
        check(vkAllocateCommandBuffers(device_, &buffer, &command_), "allocate command buffer");
        VkSemaphoreCreateInfo semaphore{};
        semaphore.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        check(vkCreateSemaphore(device_, &semaphore, nullptr, &acquired_),
              "create acquire semaphore");
        check(vkCreateSemaphore(device_, &semaphore, nullptr, &finished_),
              "create present semaphore");
        (void)recreate();
    } catch (...) {
        close();
        throw;
    }
}

void Vulkan::releaseSwapchain() noexcept {
    if (staging_)
        vkDestroyBuffer(device_, staging_, nullptr);
    if (memory_)
        vkFreeMemory(device_, memory_, nullptr);
    if (swapchain_)
        vkDestroySwapchainKHR(device_, swapchain_, nullptr);
    staging_ = VK_NULL_HANDLE;
    memory_ = VK_NULL_HANDLE;
    swapchain_ = VK_NULL_HANDLE;
    images_.clear();
}

void Vulkan::close() noexcept {
    if (device_) {
        vkDeviceWaitIdle(device_);
        releaseSwapchain();
        if (acquired_)
            vkDestroySemaphore(device_, acquired_, nullptr);
        if (finished_)
            vkDestroySemaphore(device_, finished_, nullptr);
        if (pool_)
            vkDestroyCommandPool(device_, pool_, nullptr);
        vkDestroyDevice(device_, nullptr);
    }
    if (surface_)
        vkDestroySurfaceKHR(instance_, surface_, nullptr);
    if (instance_)
        vkDestroyInstance(instance_, nullptr);
    device_ = VK_NULL_HANDLE;
    physical_ = VK_NULL_HANDLE;
    queue_ = VK_NULL_HANDLE;
    instance_ = VK_NULL_HANDLE;
    surface_ = VK_NULL_HANDLE;
    pool_ = VK_NULL_HANDLE;
    command_ = VK_NULL_HANDLE;
    acquired_ = VK_NULL_HANDLE;
    finished_ = VK_NULL_HANDLE;
    host_.close();
}

std::vector<Input> Vulkan::poll() {
    return host_.poll();
}

bool Vulkan::recreate() {
    int width = 0, height = 0;
    SDL_Vulkan_GetDrawableSize(host_.window(), &width, &height);
    if (width < 1 || height < 1)
        return false;
    check(vkDeviceWaitIdle(device_), "wait before resize");
    releaseSwapchain();
    VkSurfaceCapabilitiesKHR caps{};
    check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_, surface_, &caps),
          "surface capabilities");
    if (!(caps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT))
        throw std::runtime_error("Vulkan surface lacks transfer-destination support");
    std::uint32_t count = 0;
    check(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_, surface_, &count, nullptr),
          "surface formats");
    std::vector<VkSurfaceFormatKHR> formats(count);
    check(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_, surface_, &count, formats.data()),
          "surface formats");
    auto found = std::find_if(formats.begin(), formats.end(), [](auto f) {
        return (f.format == VK_FORMAT_B8G8R8A8_UNORM || f.format == VK_FORMAT_R8G8B8A8_UNORM) &&
               f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    });
    if (found == formats.end())
        throw std::runtime_error("Vulkan requires an unorm BGRA/RGBA surface");
    format_ = found->format;
    extent_ = caps.currentExtent;
    if (extent_.width == std::numeric_limits<std::uint32_t>::max())
        extent_ = {std::clamp(static_cast<std::uint32_t>(width), caps.minImageExtent.width,
                              caps.maxImageExtent.width),
                   std::clamp(static_cast<std::uint32_t>(height), caps.minImageExtent.height,
                              caps.maxImageExtent.height)};
    std::uint32_t images = caps.minImageCount + 1;
    if (caps.maxImageCount && images > caps.maxImageCount)
        images = caps.maxImageCount;
    VkSwapchainCreateInfoKHR swap{};
    swap.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swap.surface = surface_;
    swap.minImageCount = images;
    swap.imageFormat = format_;
    swap.imageColorSpace = found->colorSpace;
    swap.imageExtent = extent_;
    swap.imageArrayLayers = 1;
    swap.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    swap.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swap.preTransform = caps.currentTransform;
    for (auto alpha :
         {VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
          VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR, VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR})
        if (caps.supportedCompositeAlpha & alpha) {
            swap.compositeAlpha = alpha;
            break;
        }
    swap.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    swap.clipped = VK_TRUE;
    check(vkCreateSwapchainKHR(device_, &swap, nullptr, &swapchain_), "create swapchain");
    check(vkGetSwapchainImagesKHR(device_, swapchain_, &count, nullptr), "swapchain images");
    images_.resize(count);
    check(vkGetSwapchainImagesKHR(device_, swapchain_, &count, images_.data()), "swapchain images");
    VkBufferCreateInfo buffer{};
    buffer.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer.size = static_cast<VkDeviceSize>(extent_.width) * extent_.height * 4;
    buffer.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    buffer.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    check(vkCreateBuffer(device_, &buffer, nullptr, &staging_), "create staging buffer");
    VkMemoryRequirements requirements{};
    vkGetBufferMemoryRequirements(device_, staging_, &requirements);
    VkPhysicalDeviceMemoryProperties properties{};
    vkGetPhysicalDeviceMemoryProperties(physical_, &properties);
    std::uint32_t index = properties.memoryTypeCount;
    for (std::uint32_t i = 0; i < properties.memoryTypeCount; ++i)
        if ((requirements.memoryTypeBits & (1U << i)) &&
            (properties.memoryTypes[i].propertyFlags &
             (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) ==
                (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
            index = i;
            break;
        }
    if (index == properties.memoryTypeCount)
        throw std::runtime_error("Vulkan: no coherent host-visible staging memory");
    VkMemoryAllocateInfo allocation{};
    allocation.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocation.allocationSize = requirements.size;
    allocation.memoryTypeIndex = index;
    check(vkAllocateMemory(device_, &allocation, nullptr, &memory_), "allocate staging memory");
    check(vkBindBufferMemory(device_, staging_, memory_, 0), "bind staging memory");
    return true;
}

void Vulkan::record(std::uint32_t index) {
    check(vkResetCommandBuffer(command_, 0), "reset command buffer");
    VkCommandBufferBeginInfo begin{};
    begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    check(vkBeginCommandBuffer(command_, &begin), "begin commands");
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = images_[index];
    barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vkCmdPipelineBarrier(command_, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    VkBufferImageCopy region{};
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.imageExtent = {extent_.width, extent_.height, 1};
    vkCmdCopyBufferToImage(command_, staging_, images_[index], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1, &region);
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = 0;
    vkCmdPipelineBarrier(command_, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1,
                         &barrier);
    check(vkEndCommandBuffer(command_), "end commands");
}

void Vulkan::draw(const Frame &frame) {
    if (!device_)
        throw std::runtime_error("Vulkan display is closed");
    int width = 0, height = 0;
    SDL_Vulkan_GetDrawableSize(host_.window(), &width, &height);
    if (width < 1 || height < 1)
        return;
    if (!swapchain_ || static_cast<unsigned>(width) != extent_.width ||
        static_cast<unsigned>(height) != extent_.height)
        if (!recreate())
            return;
    std::uint32_t image = 0;
    auto status =
        vkAcquireNextImageKHR(device_, swapchain_, UINT64_MAX, acquired_, VK_NULL_HANDLE, &image);
    if (status == VK_ERROR_OUT_OF_DATE_KHR) {
        (void)recreate();
        return;
    }
    if (status != VK_SUBOPTIMAL_KHR)
        check(status, "acquire image");
    canvas_.render(frame);
    auto pixels = canvas_.scaled(static_cast<int>(extent_.width), static_cast<int>(extent_.height));
    if (format_ == VK_FORMAT_R8G8B8A8_UNORM)
        for (auto &p : pixels)
            p = (p & 0xff00ff00U) | ((p & 0xff0000U) >> 16U) | ((p & 0xffU) << 16U);
    void *mapped = nullptr;
    check(vkMapMemory(device_, memory_, 0, pixels.size() * 4, 0, &mapped), "map staging buffer");
    std::memcpy(mapped, pixels.data(), pixels.size() * 4);
    vkUnmapMemory(device_, memory_);
    record(image);
    VkPipelineStageFlags stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    VkSubmitInfo submit{};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.waitSemaphoreCount = 1;
    submit.pWaitSemaphores = &acquired_;
    submit.pWaitDstStageMask = &stage;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &command_;
    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &finished_;
    check(vkQueueSubmit(queue_, 1, &submit, VK_NULL_HANDLE), "submit frame");
    VkPresentInfoKHR present{};
    present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present.waitSemaphoreCount = 1;
    present.pWaitSemaphores = &finished_;
    present.swapchainCount = 1;
    present.pSwapchains = &swapchain_;
    present.pImageIndices = &image;
    status = vkQueuePresentKHR(queue_, &present);
    check(vkQueueWaitIdle(queue_), "wait for presentation");
    if (status == VK_ERROR_OUT_OF_DATE_KHR || status == VK_SUBOPTIMAL_KHR) {
        (void)recreate();
        return;
    }
    check(status, "present frame");
}
} // namespace arcade::bonus
ARCADE_PLUGIN(arcade::bonus::Vulkan, arcade::PluginKind::Graphics, "vulkan",
              "VULKAN / TRANSFER PRESENTATION")
