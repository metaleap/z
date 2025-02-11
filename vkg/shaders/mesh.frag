#version 460

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_ray_query : require
#extension GL_EXT_ray_flags_primitive_culling : require

layout(set = 0, binding = 0) uniform SceneData {
  mat4 view;
  mat4 proj;
  mat4 viewProj;
  vec4 ambientColor;
  vec4 sunlightDirectionAndPower;
  vec4 sunlightColor;
} sceneData;

layout(set = 1, binding = 0) uniform GltfMaterialData {
  vec4 colorFactors;
  vec4 metalRoughFactors;
} materialData;

layout(set = 1, binding = 1) uniform sampler2D colorTex;

layout(set = 1, binding = 2) uniform sampler2D metalRoughTex;

// #include "mesh.incl.glsl"

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inUv;

layout(location = 0) out vec4 outFragColor;

void main() {
  float light = max(0.01f, dot(inNormal, sceneData.sunlightDirectionAndPower.xyz));
  vec3 color = inColor * texture(colorTex, inUv).rgb;
  vec3 ambient = color * sceneData.ambientColor.rgb;
  outFragColor = vec4(ambient + (color * light * sceneData.sunlightDirectionAndPower.w), 1.0f);
}
