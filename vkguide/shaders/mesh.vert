#version 460

#extension GL_EXT_buffer_reference : require
#extension GL_GOOGLE_include_directive : require

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

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec3 outColor;
layout(location = 2) out vec2 outUv;

struct Vertex {
  vec3 position;
  float uvX;
  vec3 normal;
  float uvY;
  vec4 color;
};

layout(buffer_reference, std430) readonly buffer VertexBuffer {
  Vertex vertices[];
};

layout(push_constant) uniform constants {
  mat4 renderMatrix;
  VertexBuffer vertexBuffer;
} pushConstants;


void main() {
  Vertex vert = pushConstants.vertexBuffer.vertices[gl_VertexIndex];
  vec4 pos = vec4(vert.position, 1.0f);
  gl_Position = sceneData.viewProj * pushConstants.renderMatrix * pos;
  outNormal = (pushConstants.renderMatrix * vec4(vert.normal, 0.0f)).xyz;
  outColor = vert.color.rgb * materialData.colorFactors.rgb;
  outUv.x = vert.uvX;
  outUv.y = vert.uvY;
}
