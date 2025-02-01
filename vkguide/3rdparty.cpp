#include <SDL3/SDL_events.h>
#define VMA_IMPLEMENTATION
#include "./vkguide.h"

#include "../3rdparty/ocornut_imgui/imgui.h"
#include "../3rdparty/ocornut_imgui/backends/imgui_impl_sdl3.h"
#include "../3rdparty/ocornut_imgui/backends/imgui_impl_vulkan.h"


extern "C" {
void cppImguiShutdown() {
  ImGui_ImplVulkan_Shutdown();
}


void cppImguiProcessEvent(SDL_Event* evt) {
  ImGui_ImplSDL3_ProcessEvent(evt);
}


void cppImguiRender() {
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();
  ImGui::ShowDemoWindow();
  ImGui::Render();
}


void cppImguiInit(SDL_Window* window, VkInstance instance, VkPhysicalDevice gpu, VkDevice device, VkQueue queue,
                  VkDescriptorPool pool, VkFormat swapchainImageFormat) {
  ImGui::CreateContext();
  ImGui_ImplSDL3_InitForVulkan(window);
  ImGui_ImplVulkan_InitInfo init {
      .Instance                    = instance,
      .PhysicalDevice              = gpu,
      .Device                      = device,
      .Queue                       = queue,
      .DescriptorPool              = pool,
      .MinImageCount               = 3,
      .ImageCount                  = 3,
      .MSAASamples                 = VK_SAMPLE_COUNT_1_BIT,
      .UseDynamicRendering         = true,
      .PipelineRenderingCreateInfo = {.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
                                      .colorAttachmentCount    = 1,
                                      .pColorAttachmentFormats = &swapchainImageFormat}
  };
  SDL_CHECK(ImGui_ImplVulkan_Init(&init) && ImGui_ImplVulkan_CreateFontsTexture());
}
}
