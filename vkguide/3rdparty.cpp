#define VMA_IMPLEMENTATION
#include "./vkguide.h"

#include "../3rdparty/ocornut_imgui/imgui.h"
#include "../3rdparty/ocornut_imgui/backends/imgui_impl_sdl3.h"
#include "../3rdparty/ocornut_imgui/backends/imgui_impl_vulkan.h"

extern VulkanEngine vke;


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
  if (ImGui::Begin("background")) {
    ComputeShaderEffect& selected = vke.bgEffects[vke.bgEffectCurIdx];
    ImGui::Text("Selected effect: %s", selected.name);
    ImGui::SliderInt("Effect Index", &vke.bgEffectCurIdx, 0, ARR_LEN(vke.bgEffects) - 1);
    ImGui::InputFloat4("data1", (float*) &selected.pushData.data1);
    ImGui::InputFloat4("data2", (float*) &selected.pushData.data2);
    ImGui::InputFloat4("data3", (float*) &selected.pushData.data3);
    ImGui::InputFloat4("data4", (float*) &selected.pushData.data4);
  }
  ImGui::End();

  ImGui::Render();
}


void cppImguiDraw(VkCommandBuffer cmdBuf) {
  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmdBuf);
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
  SDL_CHECK(ImGui_ImplVulkan_Init(&init));
  SDL_CHECK(ImGui_ImplVulkan_CreateFontsTexture());
}
}
