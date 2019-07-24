/////////////////////////////////////
//following the niagara tutorial
/////////////////////////////////////

#include "renderer_vulkan.h"

#include "vulkan/vulkan.h"
#include "dep/glfw3.h"

#include <assert.h>


#define VK_CHECK(call) \
        do{\
          VkResult result = call;\
          assert(result == VK_SUCCESS);\
        } while (0)

#define ARRAYSIZE(array)\
        sizeof(array) / sizeof(array[0])


void brs::renderer::vulkan::RendererVulkan::main()
{
  int testvar = 4;
  int rc = glfwInit();
  assert(rc);

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
    VK_KHR_SURFACE_EXTENSION_NAME
  };

  createInfo.ppEnabledExtensionNames = extensions;
  createInfo.enabledLayerCount = ARRAYSIZE( extensions );

  VkInstance instance = 0;

  VK_CHECK( vkCreateInstance( &createInfo, 0, &instance ) );



  GLFWwindow* window = glfwCreateWindow(1024, 768, "brewing_station_vulkan", 0, 0);


  while ( !glfwWindowShouldClose( window ) )
  {
    glfwPollEvents();
  }
}
