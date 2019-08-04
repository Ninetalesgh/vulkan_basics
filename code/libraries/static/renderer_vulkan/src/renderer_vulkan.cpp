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

#include <algorithm>
#include <malloc.h>

#define VK_CHECK(call) \
        do{\
          VkResult result = call;\
          assert(result == VK_SUCCESS);\
        } while (0)

#ifndef ARRAYSIZE
  #define ARRAYSIZE(_array) sizeof(_array) / sizeof(_array[0])
#endif

#define STACK_ALLOCATE(_typename, _size) (_typename*)_malloca((u32)_size * sizeof( _typename) )

#pragma warning(disable:6001)
#pragma warning(disable:6011)


static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
  VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
  VkDebugUtilsMessageTypeFlagsEXT messageType,
  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
  void* pUserData )
{
  char const* errorType = 
    (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT )
    ? "ERROR"
    : ( messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT )
      ? "WARNING"
      : "INFO";
  char message[4096];
  printf( "%s: %s\n==========================\n", errorType, pCallbackData->pMessage );
  snprintf( message, ARRAYSIZE(message), "Vulkan-%s: %s\n==========================\n", errorType, pCallbackData->pMessage );

#ifdef _WIN32
  OutputDebugStringA( message );
#endif
  //if ( messageSeverity ^ VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT )
 //  assert( !"Validation error!" );

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

u32 GetGraphicsFamilyIndex( VkPhysicalDevice physicalDevice )
{
  u32 queueFamilyPropertyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties( physicalDevice, &queueFamilyPropertyCount, 0 );

  VkQueueFamilyProperties* queueFamilyProperties = STACK_ALLOCATE( VkQueueFamilyProperties, queueFamilyPropertyCount );

  vkGetPhysicalDeviceQueueFamilyProperties( physicalDevice, &queueFamilyPropertyCount, queueFamilyProperties );

  for ( u32 i = 0; i < queueFamilyPropertyCount; ++i )
  {
    if ( queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT )
      return i;
  }

  assert( 0 );

  //TODO: this can be used in FetchPhysicalDevice to fetch rasterization-capable device
  return VK_QUEUE_FAMILY_IGNORED;
}

bool SupportsPresentation( VkPhysicalDevice physicalDevice, u32 familyIndex )
{
#if defined(VK_USE_PLATFORM_WIN32_KHR)
  return vkGetPhysicalDeviceWin32PresentationSupportKHR( physicalDevice, familyIndex );
#endif
  assert( 0 );
  return false;
}

VkPhysicalDevice FetchPhysicalDevice( VkPhysicalDevice* physicalDevices, u32 physicalDeviceCount )
{
  VkPhysicalDevice discreteDevice = 0;
  VkPhysicalDevice fallbackDevice = 0;
  //printf( "Available GPUs:\n");

  for ( u32 i = 0; i < physicalDeviceCount; ++i )
  {
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties( physicalDevices[i], &props );

   // printf( " - GPU%d: %s\n", i, props.deviceName );

    u32 familyIndex = GetGraphicsFamilyIndex( physicalDevices[i] );
    if ( familyIndex == VK_QUEUE_FAMILY_IGNORED )
      continue;

    if ( !SupportsPresentation( physicalDevices[i], familyIndex ) )
      continue;

    if ( !discreteDevice && props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU )
    {
      discreteDevice = physicalDevices[i];
    }

    if ( !fallbackDevice )
    {
      fallbackDevice = physicalDevices[i];
    }
  }
  VkPhysicalDevice result = discreteDevice ? discreteDevice : fallbackDevice;
  if ( result )
  {
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties( result, &props );
    printf( "Selected GPU: %s\n", props.deviceName );
  }
  else
  {
    printf( "ERROR: No GPUs found\n" );
  }

  return result;
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


VkFormat GetSwapchainFormat( VkPhysicalDevice physicalDevice, VkSurfaceKHR surface )
{
  u32 surfaceFormatCount = 0;
  VK_CHECK( vkGetPhysicalDeviceSurfaceFormatsKHR( physicalDevice, surface, &surfaceFormatCount, 0 ) );

  VkSurfaceFormatKHR* surfaceFormats = STACK_ALLOCATE( VkSurfaceFormatKHR, surfaceFormatCount );
  VK_CHECK( vkGetPhysicalDeviceSurfaceFormatsKHR( physicalDevice, surface, &surfaceFormatCount, surfaceFormats ) );
  assert( surfaceFormatCount > 0 );

  if ( surfaceFormatCount == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED )
    return VK_FORMAT_R8G8B8A8_UNORM;

  //for ( int i = 0; i < surfaceFormatCount; ++i )
  //{
  //  if ( surfaceFormats[i].format == VK_FORMAT_A2R10G10B10_UNORM_PACK32 || surfaceFormats[i].format == VK_FORMAT_A2B10G10R10_UNORM_PACK32 )
  //    return surfaceFormats[i].format;
  //}

  for ( int i = 0; i < surfaceFormatCount; ++i )
  {
    if ( surfaceFormats[i].format == VK_FORMAT_R8G8B8A8_UNORM || surfaceFormats[i].format == VK_FORMAT_B8G8R8A8_UNORM )
      return surfaceFormats[i].format;
  }

  return surfaceFormats->format;
}


VkSwapchainKHR CreateSwapchain( VkPhysicalDevice physicalDevice
                              , VkDevice device
                              , VkSurfaceKHR surface
                              , u32 familyIndex
                              , VkFormat swapchainFormat
                              , u32 width
                              , u32 height )
{
  VkSurfaceCapabilitiesKHR surfaceCapabilities;
  VK_CHECK( vkGetPhysicalDeviceSurfaceCapabilitiesKHR( physicalDevice, surface, &surfaceCapabilities ) );

  VkCompositeAlphaFlagBitsKHR surfaceCompositeAlpha =
    (surfaceCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)
    ? VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR
    : (surfaceCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR)
    ? VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR
    : (surfaceCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR)
    ? VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR
    : VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;

  VkSwapchainCreateInfoKHR swapchainCreateInfo = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
  swapchainCreateInfo.surface = surface;
  swapchainCreateInfo.minImageCount = std::max(2u, surfaceCapabilities.minImageCount);
  swapchainCreateInfo.imageFormat = swapchainFormat;
  swapchainCreateInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
  swapchainCreateInfo.imageExtent = surfaceCapabilities.currentExtent;
  swapchainCreateInfo.imageArrayLayers = 1;
  swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  swapchainCreateInfo.queueFamilyIndexCount = 1;
  swapchainCreateInfo.pQueueFamilyIndices = &familyIndex;
  swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
  swapchainCreateInfo.compositeAlpha = surfaceCompositeAlpha;
  swapchainCreateInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;

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


#undef CreateSemaphore
VkSemaphore CreateSemaphore( VkDevice device )
{
  VkSemaphoreCreateInfo semaphoreCreateInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };

  VkSemaphore semaphore = 0;
  VK_CHECK( vkCreateSemaphore( device, &semaphoreCreateInfo, 0, &semaphore ) );
  return semaphore;
}


VkCommandPool CreateCommandPool( VkDevice device, u32 familyIndex )
{
  VkCommandPoolCreateInfo commandPoolCreateInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
  commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
  commandPoolCreateInfo.queueFamilyIndex = familyIndex;

  VkCommandPool commandPool = 0;
  VK_CHECK( vkCreateCommandPool( device, &commandPoolCreateInfo, 0, &commandPool ) );

  return commandPool;
}

VkRenderPass CreateRenderPass( VkDevice device, VkFormat format )
{
  VkAttachmentDescription attachmentDesc[1] = {};
  attachmentDesc[0].format = format;
  attachmentDesc[0].samples = VK_SAMPLE_COUNT_1_BIT;
  attachmentDesc[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachmentDesc[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE; //depth buffer might not want this unless we use it again
  attachmentDesc[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachmentDesc[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachmentDesc[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  attachmentDesc[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentReference colorAttachments = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

  VkSubpassDescription subpass = {};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachments;

  VkRenderPassCreateInfo renderPassCreateInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
  renderPassCreateInfo.attachmentCount = ARRAYSIZE( attachmentDesc );
  renderPassCreateInfo.pAttachments = attachmentDesc;
  renderPassCreateInfo.subpassCount = 1;
  renderPassCreateInfo.pSubpasses = &subpass;

  VkRenderPass renderPass = 0;
  VK_CHECK( vkCreateRenderPass( device, &renderPassCreateInfo, 0, &renderPass ) );

  return renderPass;
}

VkFramebuffer CreateFramebuffer(  VkDevice device
                                , VkRenderPass renderPass
                                , VkImageView imageView
                                , u32 width
                                , u32 height )
{
  VkFramebufferCreateInfo framebufferCreateInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
  framebufferCreateInfo.renderPass = renderPass;
  framebufferCreateInfo.attachmentCount = 1;
  framebufferCreateInfo.pAttachments = &imageView;
  framebufferCreateInfo.width = width;
  framebufferCreateInfo.height = height;
  framebufferCreateInfo.layers = 1;

  VkFramebuffer framebuffer = 0;
  VK_CHECK( vkCreateFramebuffer( device, &framebufferCreateInfo, 0, &framebuffer ) );
  return framebuffer;
}


VkImageView CreateImageView( VkDevice device, VkImage image, VkFormat format )
{
  VkImageViewCreateInfo imageViewCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
  imageViewCreateInfo.image = image;
  imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  imageViewCreateInfo.format = format;
  imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  imageViewCreateInfo.subresourceRange.levelCount = 1;
  imageViewCreateInfo.subresourceRange.layerCount = 1;

  VkImageView imageView = 0;
  VK_CHECK( vkCreateImageView( device, &imageViewCreateInfo, 0, &imageView ) );
  return imageView;
}

VkShaderModule LoadShader( VkDevice device, char const* path )
{
  FILE* file = fopen( path, "rb" );
  assert( file );

  fseek( file, 0, SEEK_END );
  s32 length = ftell( file );
  fseek( file, 0, SEEK_SET );
  char* buffer = new char[length];
  assert( buffer );

  u64 rc = fread( buffer, 1, length, file );
  assert( rc == u64(length) );
  fclose( file );

  assert( length % 4 == 0 );

  VkShaderModuleCreateInfo shaderCreateInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
  shaderCreateInfo.codeSize = length;
  shaderCreateInfo.pCode = reinterpret_cast<u32 const*>(buffer);

  VkShaderModule shaderModule = 0;
  VK_CHECK( vkCreateShaderModule( device, &shaderCreateInfo, 0, &shaderModule ) );
  return shaderModule;
}


VkPipelineLayout CreatePipelineLayout( VkDevice device )
{
  VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };

  VkPipelineLayout layout = 0;
  VK_CHECK( vkCreatePipelineLayout( device, &pipelineLayoutCreateInfo, 0, &layout ) );
  return layout;
}

VkPipeline CreateGraphicsPipeline(  VkDevice device
                                  , VkPipelineCache pipelineCache
                                  , VkRenderPass renderPass
                                  , VkShaderModule vs
                                  , VkShaderModule fs
                                  , VkPipelineLayout pipelineLayout )
{
  VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
 
  VkPipelineShaderStageCreateInfo stages[2] = {};
  stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
  stages[0].module = vs;
  stages[0].pName = "main";
  stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  stages[1].module = fs;
  stages[1].pName = "main";

  graphicsPipelineCreateInfo.stageCount = ARRAYSIZE( stages );
  graphicsPipelineCreateInfo.pStages = stages;
  
  VkPipelineVertexInputStateCreateInfo vertexInputState = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
  graphicsPipelineCreateInfo.pVertexInputState = &vertexInputState;
  
  VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
  inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;

  VkPipelineViewportStateCreateInfo viewportState = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
  viewportState.viewportCount = 1;
  viewportState.scissorCount = 1;
  graphicsPipelineCreateInfo.pViewportState = &viewportState;

  VkPipelineRasterizationStateCreateInfo rasterizationState = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
  rasterizationState.lineWidth = 1.f;
  graphicsPipelineCreateInfo.pRasterizationState = &rasterizationState;

  VkPipelineMultisampleStateCreateInfo multisampleState = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
  multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  graphicsPipelineCreateInfo.pMultisampleState = &multisampleState;

  VkPipelineDepthStencilStateCreateInfo depthStencilState = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
  graphicsPipelineCreateInfo.pDepthStencilState = &depthStencilState;
  
  VkPipelineColorBlendAttachmentState colorAttachmentState = {};
  colorAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

  VkPipelineColorBlendStateCreateInfo colorBlendState = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
  colorBlendState.attachmentCount = 1;
  colorBlendState.pAttachments = &colorAttachmentState;
  graphicsPipelineCreateInfo.pColorBlendState = &colorBlendState;

  VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
  VkPipelineDynamicStateCreateInfo dynamicState = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
  dynamicState.dynamicStateCount = ARRAYSIZE( dynamicStates );
  dynamicState.pDynamicStates = dynamicStates;
  graphicsPipelineCreateInfo.pDynamicState = &dynamicState;

  graphicsPipelineCreateInfo.layout = pipelineLayout;
  graphicsPipelineCreateInfo.renderPass = renderPass;

  VkPipeline pipeline = 0;
  VK_CHECK( vkCreateGraphicsPipelines( device, pipelineCache, 1, &graphicsPipelineCreateInfo, 0, &pipeline ) );
  return pipeline;
}

//TODO do transitions at once
VkImageMemoryBarrier ImageBarrier( VkImage image
                                 , VkAccessFlags srcAccessMask
                                 , VkAccessFlags dstAccessMask
                                 , VkImageLayout oldLayout
                                 , VkImageLayout newLayout )
{
  VkImageMemoryBarrier result = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
  result.srcAccessMask = srcAccessMask;
  result.dstAccessMask = dstAccessMask;
  result.oldLayout = oldLayout;
  result.newLayout = newLayout;
  result.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  result.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  result.image = image;
  result.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; //TODO: depth buffer needs smth
  result.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
  result.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS; //TODOA: some android drivers don't support these constants

  return result;
}
 /* vkCmdPipelineBarrier(commandBuffer,)
}

                                 , VkPipelineStageFlags srcStageMask
                                 , VkPipelineStageFlags dstStageMask*/

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

  u32 familyIndex = GetGraphicsFamilyIndex( physicalDevice );
  assert( familyIndex != VK_QUEUE_FAMILY_IGNORED );
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

  //Surface Support
  VkBool32 presentSupported = 0;
  VK_CHECK( vkGetPhysicalDeviceSurfaceSupportKHR( physicalDevice, familyIndex, surface, &presentSupported ) );
  assert( presentSupported == VK_TRUE );


  //Swapchain
  VkFormat swapchainFormat = GetSwapchainFormat( physicalDevice, surface );
  assert( swapchainFormat );

  s32 windowWidth = 0;
  s32 windowHeight = 0;
  glfwGetWindowSize( window, &windowWidth, &windowHeight );

  VkSwapchainKHR swapchain = CreateSwapchain(physicalDevice, device, surface, familyIndex, swapchainFormat, windowWidth, windowHeight );
  assert( swapchain );
  
  //Semaphores
  VkSemaphore aquireSemaphore = CreateSemaphore( device );
  assert( aquireSemaphore );
  VkSemaphore releaseSemaphore = CreateSemaphore( device );
  assert( releaseSemaphore );

  //Queue
  VkQueue queue = 0;
  vkGetDeviceQueue( device, familyIndex, 0, &queue );

  //Render Pass
  VkRenderPass renderPass = CreateRenderPass( device, swapchainFormat );
  assert( renderPass );

  //Shaders
  VkShaderModule triangleVS = LoadShader( device, "shaders/triangle.vert.spv" );
  assert( triangleVS );

  VkShaderModule triangleFS = LoadShader( device, "shaders/triangle.frag.spv" );
  assert( triangleFS );

  VkPipelineCache pipelineCache = 0;
  VkPipelineLayout triangleLayout = CreatePipelineLayout( device );
  assert( triangleLayout );

  VkPipeline trianglePipeline = CreateGraphicsPipeline( device, pipelineCache, renderPass, triangleVS, triangleFS, triangleLayout );
  assert( trianglePipeline );

  //Images
  u32 swapchainImageCount = 0;
  VK_CHECK( vkGetSwapchainImagesKHR( device, swapchain, &swapchainImageCount, 0) );
  VkImage* swapchainImages = STACK_ALLOCATE( VkImage, swapchainImageCount );
  VK_CHECK( vkGetSwapchainImagesKHR( device, swapchain, &swapchainImageCount, swapchainImages ) );

  //Image View
  VkImageView swapchainImageViews[16];
  for ( u32 i = 0; i < swapchainImageCount; ++i )
  {
    swapchainImageViews[i] = CreateImageView( device, swapchainImages[i], swapchainFormat );
    assert( swapchainImageViews[i] );
  }

  VkFramebuffer swapchainFramebuffers[16];
  for ( u32 i = 0; i < swapchainImageCount; ++i )
  {
    swapchainFramebuffers[i] = CreateFramebuffer( device, renderPass, swapchainImageViews[i], windowWidth, windowHeight );
    assert( swapchainFramebuffers[i] );
  }
  
  //Command Pool
  VkCommandPool commandPool = CreateCommandPool( device, familyIndex );
  assert( commandPool );

  VkCommandBufferAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
  allocateInfo.commandPool = commandPool;
  allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
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

    VkCommandBufferBeginInfo commandBufferBeginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    //Begin Command Buffer
    VK_CHECK( vkBeginCommandBuffer( commandBuffer, &commandBufferBeginInfo ) );

    VkImageMemoryBarrier renderBeginBarrier = ImageBarrier( swapchainImages[imageIndex], 0
                                                          , VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
                                                          , VK_IMAGE_LAYOUT_UNDEFINED
                                                          , VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL );

    vkCmdPipelineBarrier( commandBuffer
                        , VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT
                        , VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
                        , VK_DEPENDENCY_BY_REGION_BIT
                        , 0, 0, 0, 0, 1
                        , &renderBeginBarrier );
    
    VkClearColorValue color = { 48.f/255.f, 10.f/255.f, 36.f/255.f, 1 };
    VkClearValue clearColor = { color };

   // vkCmdPipelineBarrier( commandBuffer, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT );

    //Begin Render Pass
    VkRenderPassBeginInfo renderPassBeginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.framebuffer = swapchainFramebuffers[imageIndex];
    renderPassBeginInfo.renderArea.extent.width = windowWidth;
    renderPassBeginInfo.renderArea.extent.height = windowHeight;
    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = &clearColor;
    
    vkCmdBeginRenderPass( commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE );

    VkViewport viewport = { 0, float( windowHeight ), float( windowWidth ), -float( windowHeight ), 0, 1 };
    VkRect2D scissor = { { 0, 0 }, { u32( windowWidth ), u32( windowHeight ) } };

    vkCmdSetViewport( commandBuffer, 0, 1, &viewport );
    vkCmdSetScissor( commandBuffer, 0, 1, &scissor );

    //Draw calls
    vkCmdBindPipeline( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, trianglePipeline );
    vkCmdDraw( commandBuffer, 3, 1, 0, 0 );

    //End Render Pass
    vkCmdEndRenderPass( commandBuffer );

    VkImageMemoryBarrier renderEndBarrier = ImageBarrier( swapchainImages[imageIndex]
                                                        , VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
                                                        , 0
                                                        , VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
                                                        , VK_IMAGE_LAYOUT_PRESENT_SRC_KHR );

    vkCmdPipelineBarrier( commandBuffer
                        , VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
                        , VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT
                        , VK_DEPENDENCY_BY_REGION_BIT
                        , 0, 0, 0, 0, 1
                        , &renderEndBarrier );


    //End Command Buffer
    VK_CHECK( vkEndCommandBuffer( commandBuffer ) );


    //Submit Stage
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

    //TODO: remove when he switches to desktop
    //glfwWaitEvents();

  }

  vkDestroyCommandPool( device, commandPool, 0 );

  for ( u32 i = 0; i < swapchainImageCount; ++i )
    vkDestroyFramebuffer( device, swapchainFramebuffers[i], 0 );

  for ( u32 i = 0; i < swapchainImageCount; ++i )
    vkDestroyImageView( device, swapchainImageViews[i], 0 );

  vkDestroyPipeline( device, trianglePipeline, 0 );

  vkDestroyPipelineLayout( device, triangleLayout, 0 );

  vkDestroyShaderModule( device, triangleVS, 0 );
  vkDestroyShaderModule( device, triangleFS, 0 );

  vkDestroyRenderPass( device, renderPass, 0 );

  vkDestroySemaphore( device, releaseSemaphore, 0 );
  vkDestroySemaphore( device, aquireSemaphore, 0 );

  vkDestroySwapchainKHR( device, swapchain, 0 );
  vkDestroySurfaceKHR( instance, surface, 0 );

  vkDestroyDevice( device, 0 );

  DestroyDebugUtilsMessengerEXT( instance, debugMessenger, nullptr );

  vkDestroyInstance( instance, 0 );

  glfwDestroyWindow( window );
}
