#version 460
#extension GL_EXT_buffer_reference : require


struct Vertex {
	vec3 position;
	float uv_x;
	vec3 normal;
	float uv_y;
	vec4 color;
};


layout(buffer_reference, std430) readonly buffer VertexBuffer {
	Vertex vertices[];
};
layout(push_constant) uniform constants {
	mat4 render_matrix;
	VertexBuffer vertexBuffer;
} pushConstants;


layout(location = 0) out vec3 outColor;
layout(location = 1) out vec2 outUv;


void main() {
  // load vertex data from device adress
  Vertex vert = pushConstants.vertexBuffer.vertices[gl_VertexIndex];

	gl_Position = pushConstants.render_matrix * vec4(vert.position, 1.0f);
	outColor = vert.color.rgb;
	outUv.x = vert.uv_x;
	outUv.y = vert.uv_y;
}
