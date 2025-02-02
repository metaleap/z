#include <SDL3/SDL_events.h>
#include <cstdint>
#define VMA_IMPLEMENTATION
#include "./vkguide.h"

#include <unordered_map>
#include <filesystem>

#include "../3rdparty/ocornut_imgui/imgui.h"
#include "../3rdparty/ocornut_imgui/backends/imgui_impl_sdl3.h"
#include "../3rdparty/ocornut_imgui/backends/imgui_impl_vulkan.h"
#include "../3rdparty/spnda_fastgltf/include/fastgltf/core.hpp"
#include "../3rdparty/spnda_fastgltf/include/fastgltf/tools.hpp"

extern VulkanEngine vke;


extern "C" {
List cppLoadMeshes(char* filePath) {
  List                  ret       = {};
  std::filesystem::path file_path = filePath;
  auto                  data      = fastgltf::GltfDataBuffer::FromPath(file_path);
  if (data.error() != fastgltf::Error::None)
    return ret;
  fastgltf::Parser parser;
  auto             asset = parser.loadGltfBinary(data.get(), file_path.parent_path(), fastgltf::Options::None);
  if (auto error = asset.error(); error != fastgltf::Error::None)
    return ret;

  ret = List_new(8);
  std::vector<uint32_t> indices;
  std::vector<Vertex>   vertices;
  for (fastgltf::Mesh& mesh : asset->meshes) {
    MeshAsset newmesh;
    newmesh.name = mesh.name.c_str();
    indices.clear();
    vertices.clear();
    for (auto&& p : mesh.primitives) {
      GeoSurface newSurface  = {.idxStart = (Uint32) indices.size(),
                                .count    = (Uint64) asset->accessors[p.indicesAccessor.value()].count};
      size_t     initial_vtx = vertices.size();
      // load indexes
      {
        fastgltf::Accessor& indexaccessor = asset->accessors[p.indicesAccessor.value()];
        indices.reserve(indices.size() + indexaccessor.count);
        fastgltf::iterateAccessor<std::uint32_t>(
            asset.get(), indexaccessor,
            [&](std::uint32_t idx) { indices.push_back(idx + (uint32_t) initial_vtx); },
            fastgltf::DefaultBufferDataAdapter());
      }
      // load vertex positions
      {
        fastgltf::Accessor& posAccessor = asset->accessors[p.findAttribute("POSITION")->second];
        vertices.resize(vertices.size() + posAccessor.count);
        fastgltf::iterateAccessorWithIndex<vec3s>(asset, posAccessor, [&](vec3s v, size_t index) {
          Vertex newvtx;
          newvtx.position               = v;
          newvtx.normal                 = {1, 0, 0};
          newvtx.color                  = glm::vec4 {1.f};
          newvtx.uv_x                   = 0;
          newvtx.uv_y                   = 0;
          vertices[initial_vtx + index] = newvtx;
        });
      }
    }
  }
  return ret;
}

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


void cppImguiInit(SDL_Window* window, VkInstance instance, VkPhysicalDevice gpu, VkDevice device,
                  VkQueue queue, VkDescriptorPool pool, VkFormat swapchainImageFormat) {
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
      .PipelineRenderingCreateInfo = {.sType                = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
                                      .colorAttachmentCount = 1,
                                      .pColorAttachmentFormats = &swapchainImageFormat}
  };
  SDL_CHECK(ImGui_ImplVulkan_Init(&init));
  SDL_CHECK(ImGui_ImplVulkan_CreateFontsTexture());
}
}
