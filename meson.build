project(
    'a0',
    ['c'],
    version: '0.0.1',
    default_options: ['cpp_std=c++20', 'c_std=c23', 'cpp_rtti=false', 'cpp_eh=none'],
)

add_global_arguments(
    '-gsplit-dwarf',
    '-fms-extensions',
    '-msse2',
    '-mfpmath=sse',
    '-march=native',
    language: ['c'],
)

add_global_link_arguments('-lm', language: ['c'])

dep_sdl2 = dependency('SDL2', include_type: 'system', static: false)
dep_vulkan = dependency('vulkan', include_type: 'system', static: false)
dep_pthread = dependency('threads', include_type: 'system', static: false)

executable(
    'fged.exec',
    dependencies: [dep_vulkan, dep_sdl2, dep_pthread],
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
    dependencies: [dep_vulkan, dep_sdl2, dep_pthread],
    link_language: 'c',
    sources: [
        'vkguide/main.c',
        'vkguide/vk_descriptors.c',
        'vkguide/vk_engine.c',
        'vkguide/vk_images.c',
        'vkguide/vk_initializers.c',
        'vkguide/vk_loader.c',
        'vkguide/vk_pipelines.c',
        'vkguide/vk_types.c',
    ],
)
