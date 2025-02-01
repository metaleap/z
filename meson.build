project(
    'a0',
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
    '-Wno-incompatible-pointer-types', # no more `const T*` vs `T*`
    language: ['c', 'cpp'],
)
add_global_link_arguments(
    '-lm',
    language: ['c', 'cpp'],
)

dep_pthread = dependency('threads', include_type: 'system', static: false)
dep_sdl3 = dependency('SDL3', include_type: 'system', static: false)
dep_vulkan = dependency('vulkan', include_type: 'system', static: false)
dep_glslang = dependency('glslang', include_type: 'system', static: false)
dep_glslang2 = meson.get_compiler('c').find_library('glslang-default-resource-limits', static: false)

executable(
    'fged.exec',
    dependencies: [dep_vulkan, dep_sdl3, dep_pthread],
    link_language: 'c',
    sources: [
        'fged/main.c',
        'fged/vec3.c',
        'fged/mat3.c',
        'fged/vec4.c',
        'fged/mat4.c',
    ],
)

executable(
    'vkguide.exec',
    dependencies: [dep_glslang2, dep_glslang, dep_vulkan, dep_sdl3, dep_pthread],
    link_language: 'cpp',
    implicit_include_directories: true,
    include_directories: ['3rdparty/ocornut_imgui', '3rdparty/ocornut_imgui/backends'],
    sources: [
        'vkguide/main.c',
        'vkguide/vk_descriptors.c',
        'vkguide/vk_engine.c',
        'vkguide/vk_images.c',
        'vkguide/vk_initializers.c',
        'vkguide/vk_loader.c',
        'vkguide/vk_pipelines.c',
        'vkguide/vk_types.c',
        'vkguide/3rdparty.cpp',
        '3rdparty/ocornut_imgui/imgui.cpp',
        '3rdparty/ocornut_imgui/imgui_draw.cpp',
        '3rdparty/ocornut_imgui/imgui_tables.cpp',
        '3rdparty/ocornut_imgui/imgui_widgets.cpp',
        '3rdparty/ocornut_imgui/backends/imgui_impl_sdl3.cpp',
        '3rdparty/ocornut_imgui/backends/imgui_impl_vulkan.cpp',
    ],
)
