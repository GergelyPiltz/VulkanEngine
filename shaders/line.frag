#version 450

layout(binding = 1) uniform sampler2D texSampler[];

layout (location = 0) out vec4 outColor;

struct PointLight {
	vec4 position;
	vec4 color;
};

layout(set = 0, binding = 0) uniform GlobalUBO {
	mat4 projection;
	mat4 view;
	mat4 invView;
	vec4 ambientLight;
	PointLight  pointLights[10];
	int numLights;
} ubo;

layout(push_constant) uniform Push {
	mat4 modelMatrix;
	mat3x4 normalMatrix;
	int textureIndex;
} push;

void main() {
	outColor = vec4( 1, 1, 1, 1 );
}