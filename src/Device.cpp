#include "Device.h"

#include "VulkanHandle.h"

#include "SwapChainHandler.h"

Device::Device(std::initializer_list<const char*> deviceExtensions,std::initializer_list<const char*> validationLayers, VkInstance instance, VkSurfaceKHR surface)
{
  DeviceSetup setup{deviceExtensions, validationLayers, instance, surface};
  PickPhysicalDevice(setup);
  CreateLogicalDevice(setup);
}

void Device::PickPhysicalDevice(DeviceSetup& setup)
{
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(setup.instance, &deviceCount, nullptr);
  if(deviceCount == 0)
    throw std::runtime_error("Failed to find GPU with Vulkan support");

  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(setup.instance, &deviceCount, devices.data());

  for(auto&& device : devices)
  {
    if(IsDeviceSuitable(setup, device))
    {
      physicalDevice = device;
      break;
    }
  }
  if(physicalDevice == VK_NULL_HANDLE)
    throw std::runtime_error("Failed to find GPU with support of all needed operations");
  PrintPhysicalDeviceName(physicalDevice);
}

bool Device::IsDeviceSuitable(DeviceSetup& setup, VkPhysicalDevice device)
{
  QueueFamilyIndices indices = VulkanHandle::FindQueueFamilies(device, setup.surface);
  bool extensionSupported = CheckDeviceExtensionSupport(setup, device);
  bool swapChainAdequate = false;
  if(extensionSupported)
  {
    SwapChainSupportDetails swapChainSupport = SwapChainHandler::QuerySwapChainSupport(setup.surface, device);
    swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
  }
  VkPhysicalDeviceFeatures supportedFeatures;
  vkGetPhysicalDeviceFeatures(device, &supportedFeatures);
  return indices.IsComplete() && extensionSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

bool Device::CheckDeviceExtensionSupport(DeviceSetup& setup, VkPhysicalDevice device)
{
  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

  std::set<std::string> requiredExtensions(setup.deviceExtensions.begin(), setup.deviceExtensions.end());

  for(auto&& extension : availableExtensions)
    requiredExtensions.erase(extension.extensionName);
  return requiredExtensions.empty();
}

void Device::PrintPhysicalDeviceName(VkPhysicalDevice device)
{
  VkPhysicalDeviceProperties deviceProperties;
  vkGetPhysicalDeviceProperties(device, &deviceProperties);

  uint32_t version;
  vkEnumerateInstanceVersion(&version);
  std::cout << "INFO: Vulkan v" <<
    VK_VERSION_MAJOR(deviceProperties.apiVersion) << "." <<
    VK_VERSION_MINOR(deviceProperties.apiVersion) << "." <<
    VK_VERSION_PATCH(deviceProperties.apiVersion) << std::endl;

  std::cout << "INFO: " <<
    deviceProperties.deviceName << " v" <<
    VK_VERSION_MAJOR(deviceProperties.driverVersion) << "." <<
    VK_VERSION_MINOR(deviceProperties.driverVersion) << "." <<
    VK_VERSION_PATCH(deviceProperties.driverVersion) << std::endl;
}

void Device::CreateLogicalDevice(DeviceSetup& setup)
{
  QueueFamilyIndices indices = VulkanHandle::FindQueueFamilies(this, setup.surface);
  float queuePriority = 1.0f;
  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

  std::set<uint32_t> uniqueQueueFamilies = {
    indices.graphicsFamily.value(),
    indices.presentFamily.value(),
  };
  for(uint32_t queueFamily : uniqueQueueFamilies)
  {
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(queueCreateInfo);
  }

  VkPhysicalDeviceFeatures deviceFeatures = {};
  deviceFeatures.samplerAnisotropy = VK_TRUE;
  VkDeviceCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
  createInfo.pQueueCreateInfos = queueCreateInfos.data();
  createInfo.pEnabledFeatures = &deviceFeatures;
  createInfo.enabledExtensionCount = static_cast<uint32_t>(setup.deviceExtensions.size());
  createInfo.ppEnabledExtensionNames = setup.deviceExtensions.data();
#ifdef _DEBUG
  createInfo.enabledLayerCount = static_cast<uint32_t>(setup.validationLayers.size());
  createInfo.ppEnabledLayerNames = setup.validationLayers.data();
#else
  createInfo.enabledLayerCount = 0;
#endif

  if(vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
    throw std::runtime_error("Failed to create logical device");

  vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
  vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}
