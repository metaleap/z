#include "./vkguide.h"



bool vlkLoadShaderModule(char* filePath, VkDevice device, VkShaderModule* retShaderModule) {
  size_t  fileSize;
  Uint32* data = SDL_LoadFile(filePath, &fileSize);
  SDL_CHECK(data);
  VkShaderModuleCreateInfo create = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, .pCode = data, .codeSize = fileSize};
  return vkCreateShaderModule(device, &create, nullptr, retShaderModule);
}
