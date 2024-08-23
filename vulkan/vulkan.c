#include "vulkan.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

static void init_vk() {
  VkResult res = vkCreateInstance(
      &(VkInstanceCreateInfo){
          .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
          .pApplicationInfo =
              &(VkApplicationInfo){
                  .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                  .pApplicationName = "vkcube",
                  .apiVersion = VK_MAKE_VERSION(1, 1, 0),
              },
          .enabledExtensionCount = extension ? 2 : 0,
          .ppEnabledExtensionNames =
              (const char *[2]){
                  VK_KHR_SURFACE_EXTENSION_NAME,
                  extension,
              },
      },
      NULL, &vc->instance);
  fail_if(res != VK_SUCCESS, "Failed to create Vulkan instance.\n");

  uint32_t count;
  res = vkEnumeratePhysicalDevices(vc->instance, &count, NULL);
  fail_if(res != VK_SUCCESS || count == 0, "No Vulkan devices found.\n");
  VkPhysicalDevice pd[count];
  vkEnumeratePhysicalDevices(vc->instance, &count, pd);
  vc->physical_device = pd[0];
  printf("%d physical devices\n", count);

  void init_vk() {}
