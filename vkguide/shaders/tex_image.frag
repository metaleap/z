#version 460

layout(set = 0, binding = 0) uniform sampler2D unifTex;

layout(location = 0) in vec3 inColor;
layout(location = 1) in vec2 inUv;

layout(location = 0) out vec4 outColor;


void main() {
  outColor = texture(unifTex, inUv);
}
