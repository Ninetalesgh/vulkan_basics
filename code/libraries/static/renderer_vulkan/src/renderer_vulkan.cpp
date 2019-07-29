/////////////////////////////////////
//following the niagara tutorial
/////////////////////////////////////


#include "renderer_vulkan.h"

#include "vulkan/vulkan.h"
#include "dep/glfw3.h"
#include "dep/glfw3native.h"

#include <assert.h>
#include <common.h>

#include <stdio.h>

#define VK_CHECK(call) \
        do{\
          VkResult result = call;\
          assert(result == VK_SUCCESS);\
        } while (0)

#define ARRAYSIZE(array)\
        sizeof(array) / sizeof(array[0])


static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
  VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
  VkDebugUtilsMessageTypeFlagsEXT messageType,
  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
  void* pUserData ) {

  printf( "vulkan validation layer: %s\n================\n", pCallbackData->pMessage );

  return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT( VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger ) {
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr( instance, "vkCreateDebugUtilsMessengerEXT" );
  if ( func != nullptr ) {
    return func( instance, pCreateInfo, pAllocator, pDebugMessenger );
  }
  else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

void DestroyDebugUtilsMessengerEXT( VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator ) {
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr( instance, "vkDestroyDebugUtilsMessengerEXT" );
  if ( func != nullptr ) {
    func( instance, debugMessenger, pAllocator );
  }
}

VkPhysicalDevice FetchPhysicalDevice( VkPhysicalDevice* physicalDevices, u32 physicalDeviceCount )
{
  for ( u32 i = 0; i < physicalDeviceCount; ++i )
  {
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties( physicalDevices[i], &props );
    if ( props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU )
    {
      printf( "Discrete GPU found.\n" );
      return physicalDevices[i];
    }

    if ( physicalDeviceCount > 0 )
    {
      VkPhysicalDeviceProperties props;
      vkGetPhysicalDeviceProperties( physicalDevices[0], &props );
      printf( "No discrete GPU found, fetching fallback.\n" );
      return physicalDevices[0];
    }

    printf( "No physical devices available!\n" );
    return 0;
  }
}

VkInstance CreateInstance()
{
  //TODO use vkEnumerateInstanceVersion

  VkApplicationInfo appInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
  appInfo.apiVersion = VK_API_VERSION_1_1;

  VkInstanceCreateInfo createInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
  createInfo.pApplicationInfo = &appInfo;

#ifdef    _DEBUG
  char const* debugLayers[] =
  {
    "VK_LAYER_LUNARG_standard_validation"
  };
  createInfo.ppEnabledLayerNames = debugLayers;
  createInfo.enabledLayerCount = ARRAYSIZE( debugLayers );
#endif

  char const* extensions[] =
  {
    VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef VK_USE_PLATFORM_WIN32_KHR
    VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
#ifdef _DEBUG
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
  };

  createInfo.ppEnabledExtensionNames = extensions;
  createInfo.enabledExtensionCount = ARRAYSIZE( extensions );
  VkInstance instance = 0;
  VK_CHECK( vkCreateInstance( &createInfo, 0, &instance ) );
  return instance;
}

VkDevice CreateDevice( VkInstance instance, VkPhysicalDevice physicalDevice, u32* familyIndex )
{
  *familyIndex = 0;

  float queuePriorities[] = { 1.0f };

  VkDeviceQueueCreateInfo deviceQueueInfo = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
  deviceQueueInfo.queueFamilyIndex = *familyIndex;
  deviceQueueInfo.queueCount = 1;
  deviceQueueInfo.pQueuePriorities = queuePriorities;

  char const* extensions[] =
  {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
  };

  VkDeviceCreateInfo deviceCreateInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
  deviceCreateInfo.queueCreateInfoCount = 1;
  deviceCreateInfo.pQueueCreateInfos = &deviceQueueInfo;
  deviceCreateInfo.ppEnabledExtensionNames = extensions;
  deviceCreateInfo.enabledExtensionCount = ARRAYSIZE( extensions );
  deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

  VkDevice device = 0;
  VK_CHECK( vkCreateDevice( physicalDevice, &deviceCreateInfo, 0, &device ) );

  return device;
}



VkSurfaceKHR CreateSurface(VkInstance instance, GLFWwindow* window )
{
#if defined(VK_USE_PLATFORM_WIN32_KHR)
  VkWin32SurfaceCreateInfoKHR createInfo = { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
  createInfo.hinstance = GetModuleHandle( 0 );
  createInfo.hwnd = glfwGetWin32Window( window );

  VkSurfaceKHR surface = 0;
  VK_CHECK( vkCreateWin32SurfaceKHR( instance, &createInfo, 0, &surface ) );
  return surface;
#else
#error Unsupported platform
#endif
}


VkSwapchainKHR CreateSwapchain(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, u32 familyIndex, u32 width, u32 height )
{
  VkSurfaceCapabilitiesKHR surfaceCapabilities;
  VK_CHECK( vkGetPhysicalDeviceSurfaceCapabilitiesKHR( physicalDevice, surface, &surfaceCapabilities ) );

  VkSurfaceFormatKHR surfaceFormats[16];
  u32 surfaceFormatCount = ARRAYSIZE( surfaceFormats );
  VK_CHECK( vkGetPhysicalDeviceSurfaceFormatsKHR( physicalDevice, surface, &surfaceFormatCount, surfaceFormats ) );

  VkBool32 supported;
  VK_CHECK( vkGetPhysicalDeviceSurfaceSupportKHR( physicalDevice, familyIndex, surface, &supported ) );
  assert( supported == VK_TRUE );

  VkSwapchainCreateInfoKHR swapchainCreateInfo = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
  swapchainCreateInfo.surface = surface;
  swapchainCreateInfo.minImageCount = 2;
  swapchainCreateInfo.imageFormat = surfaceFormats[0].format;
  swapchainCreateInfo.imageColorSpace = surfaceFormats[0].colorSpace;
  swapchainCreateInfo.imageExtent = surfaceCapabilities.currentExtent;
  swapchainCreateInfo.imageArrayLayers = 1;
  swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  swapchainCreateInfo.queueFamilyIndexCount = 1;
  swapchainCreateInfo.pQueueFamilyIndices = &familyIndex;
  swapchainCreateInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
  swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

  VkSwapchainKHR swapchain = 0;
  VkResult aa = vkCreateSwapchainKHR( device, &swapchainCreateInfo, 0, &swapchain );

  return swapchain;
}

VkDebugUtilsMessengerEXT CreateDebugMessenger( VkInstance instance )
{
  VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo = {};
  debugUtilsMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  debugUtilsMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  debugUtilsMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  debugUtilsMessengerCreateInfo.pfnUserCallback = debugCallback;
  debugUtilsMessengerCreateInfo.pUserData = nullptr;

  VkDebugUtilsMessengerEXT debugMessenger;
  VK_CHECK( CreateDebugUtilsMessengerEXT( instance, &debugUtilsMessengerCreateInfo, 0, &debugMessenger ) );

  return debugMessenger;
}

VkSemaphore createSemaphore( VkDevice device )
{
  VkSemaphoreCreateInfo semaphoreCreateInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };

  VkSemaphore semaphore = 0;
  VK_CHECK( vkCreateSemaphore( device, &semaphoreCreateInfo, 0, &semaphore ) );
  return semaphore;
}

VkCommandPool CreateCommandPool( VkDevice device, u32 familyIndex)
{
  VkCommandPoolCreateInfo commandPoolCreateInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
  commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
  commandPoolCreateInfo.queueFamilyIndex = familyIndex;

  VkCommandPool commandPool = 0;
  VK_CHECK( vkCreateCommandPool( device, &commandPoolCreateInfo, 0, &commandPool ) );

  return commandPool;
}

void brs::renderer::vulkan::RendererVulkan::main()
{
  int rc = glfwInit();
  assert( rc );

  //Instance
  VkInstance instance = CreateInstance();
  assert( instance );

  VkDebugUtilsMessengerEXT debugMessenger = CreateDebugMessenger( instance );
  assert( debugMessenger );

  //Physical Device
  VkPhysicalDevice physicalDevices[16];
  u32 physicalDeviceCount = ARRAYSIZE( physicalDevices );
  VK_CHECK( vkEnumeratePhysicalDevices( instance, &physicalDeviceCount, physicalDevices ) );
  
  VkPhysicalDevice physicalDevice = FetchPhysicalDevice( physicalDevices, physicalDeviceCount );
  assert( physicalDevice );

  u32 familyIndex = 0;
  //Device
  VkDevice device = CreateDevice( instance, physicalDevice, &familyIndex );
  assert( device );

  //Window
  glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
  GLFWwindow* window = glfwCreateWindow( 1024, 768, "brewing_station_vulkan", 0, 0 );
  assert( window );
  
  //Surface
  VkSurfaceKHR surface = CreateSurface( instance, window );
  assert( surface );

  //Swapchain
  s32 windowWidth = 0;
  s32 windowHeight = 0;
  glfwGetWindowSize( window, &windowWidth, &windowHeight );

  VkSwapchainKHR swapchain = CreateSwapchain(physicalDevice, device, surface, familyIndex, windowWidth, windowHeight );
  assert( swapchain );
  
  //Semaphores
  VkSemaphore aquireSemaphore = createSemaphore( device );
  assert( aquireSemaphore );
  VkSemaphore releaseSemaphore = createSemaphore( device );
  assert( releaseSemaphore );

  //Queue
  VkQueue queue = 0;
  vkGetDeviceQueue( device, familyIndex, 0, &queue );

  VkImage swapchainImages[16];
  u32 swapchainImageCount = ARRAYSIZE( swapchainImages );
  VK_CHECK( vkGetSwapchainImagesKHR( device, swapchain, &swapchainImageCount, swapchainImages ) );

  //Command Pool
  VkCommandPool commandPool = CreateCommandPool( device, familyIndex );
  assert( commandPool );

  VkCommandBufferAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
  allocateInfo.commandPool = commandPool;
  allocateInfo.level - VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocateInfo.commandBufferCount = 1;

  VkCommandBuffer commandBuffer = 0;
  vkAllocateCommandBuffers( device, &allocateInfo, &commandBuffer);

  //Run
  while ( !glfwWindowShouldClose( window ) )
  {
    glfwPollEvents();

    u32 imageIndex = 0;
    VK_CHECK( vkAcquireNextImageKHR( device, swapchain, ~0ull, aquireSemaphore, VK_NULL_HANDLE, &imageIndex ) );

    VK_CHECK( vkResetCommandPool( device, commandPool, 0 ) );

    VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VK_CHECK( vkBeginCommandBuffer( commandBuffer, &beginInfo ) );

    VkClearColorValue color = { 1,0,1,1 };
    VkImageSubresourceRange range = {  };
    range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    range.levelCount = 1;
    range.layerCount = 1;

    vkCmdClearColorImage( commandBuffer, swapchainImages[imageIndex], VK_IMAGE_LAYOUT_GENERAL, &color, 1, &range );

    VK_CHECK( vkEndCommandBuffer( commandBuffer ) );

    VkPipelineStageFlags submitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &aquireSemaphore;
    submitInfo.pWaitDstStageMask = &submitStageMask;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &releaseSemaphore;

    vkQueueSubmit( queue, 1, &submitInfo, VK_NULL_HANDLE );

    VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &releaseSemaphore;
    
    VK_CHECK( vkQueuePresentKHR( queue, &presentInfo ) );
    
    VK_CHECK( vkDeviceWaitIdle( device ) );
  }

  glfwDestroyWindow( window );
  DestroyDebugUtilsMessengerEXT( instance, debugMessenger, nullptr );

}
