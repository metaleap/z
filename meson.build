project(
    'z',
    ['c', 'cpp'],
    version: '0.0.1',
    default_options: ['cpp_std=c++20', 'c_std=c23', 'cpp_rtti=false', 'cpp_eh=none'],
    meson_version: '>=1.1',
)
add_global_arguments(
    '-gsplit-dwarf',
    '-fms-extensions',
    '-msse2',
    '-mfpmath=sse',
    '-march=native',
    '-Wno-switch',
    '-Wno-unused-variable',
    # '-Wno-incompatible-pointer-types', # note for the future: keep disabled AND keep this reminder  =)
    language: ['c', 'cpp'],
)
add_global_link_arguments(
    '-lm',
    language: ['c', 'cpp'],
)

dep_pthread = dependency('threads', include_type: 'system', static: false)
dep_sdl3 = dependency('SDL3', include_type: 'system', static: false)
dep_vulkan = dependency('vulkan', include_type: 'system', static: false)
dep_glslang = dependency('glslang', include_type: 'system')
dep_glslang_drl = meson.get_compiler('c').find_library('glslang-default-resource-limits')

dep_fastgltf = meson.get_compiler('cpp').find_library(
    'fastgltf',
    static: true,
    dirs: [meson.project_source_root() / '3rdparty/spnda_fastgltf/.build'],
)
lib_zdeps = library(
    'zdeps',
    dependencies: [
        dep_fastgltf,
        dep_glslang_drl,
        dep_glslang,
        dep_vulkan,
        dep_sdl3,
        dep_pthread,
    ],
    build_by_default: true,
    implicit_include_directories: true,
    link_language: 'cpp',
    include_directories: [
        '3rdparty/cimgui_cimgui',
        '3rdparty/GPUOpen-LibrariesAndSDKs_VulkanMemoryAllocator/include',
        '3rdparty/jkuhlmann_cgltf',
        '3rdparty/mystborn_GenericDataStructures/include',
        '3rdparty/recp_cglm/include',
        '3rdparty/spnda_fastgltf/include',
        '3rdparty/ocornut_imgui',
        '3rdparty/ocornut_imgui/backends',
    ],
    pic: true,
    sources: [
        'zdeps/impls.c',
        'zdeps/impls.cpp',
    ],
)

executable(
    'vkguide.exec',
    dependencies: [
        dep_glslang_drl,
        dep_glslang,
        dep_vulkan,
        dep_sdl3,
        dep_pthread,
    ],
    link_language: 'cpp',
    link_with: [lib_zdeps],
    implicit_include_directories: true,
    include_directories: [
        'zdeps/',
        '3rdparty/cimgui_cimgui',
        '3rdparty/jkuhlmann_cgltf',
        '3rdparty/ocornut_imgui',
        '3rdparty/ocornut_imgui/backends',
        '3rdparty/mystborn_GenericDataStructures/include',
        '3rdparty/recp_cglm/include',
        '3rdparty/GPUOpen-LibrariesAndSDKs_VulkanMemoryAllocator/include',
        '3rdparty/spnda_fastgltf/include',
    ],
    # native: true, # https://github.com/mesonbuild/meson/discussions/14218
    sources: [
        'vkguide/main.c',
        'vkguide/camera.c',
        'vkguide/materials.c',
        'vkguide/vk_engine.c',
        'vkguide/vk_descriptors.c',
        'vkguide/vk_images.c',
        'vkguide/vk_initializers.c',
        'vkguide/vk_loader.c',
        'vkguide/vk_pipelines.c',
        'vkguide/vk_scenenode.c',
        'vkguide/3rdparty.cpp',
        '3rdparty/ocornut_imgui/imgui.cpp',
        '3rdparty/ocornut_imgui/imgui_demo.cpp',
        '3rdparty/ocornut_imgui/imgui_draw.cpp',
        '3rdparty/ocornut_imgui/imgui_tables.cpp',
        '3rdparty/ocornut_imgui/imgui_widgets.cpp',
        '3rdparty/ocornut_imgui/backends/imgui_impl_sdl3.cpp',
        '3rdparty/ocornut_imgui/backends/imgui_impl_vulkan.cpp',
    ],
)
