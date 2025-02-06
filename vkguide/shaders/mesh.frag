#version 460

#extension GL_GOOGLE_include_directive : require
#include "mesh.incl.glsl"

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
