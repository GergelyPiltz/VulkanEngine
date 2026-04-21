#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec3 fragPosWorld;
layout(location = 1) in vec3 fragNormalWorld;
layout(location = 2) in vec2 fragTexCoord;

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
	vec3 diffuseLight = ubo.ambientLight.xyz * ubo.ambientLight.w;
	vec3 specularLight = vec3(0.0);
	vec3 surfaceNormal = normalize(fragNormalWorld);

	vec3 cameraPosWorld = ubo.invView[3].xyz;
	vec3 viewDirection = normalize(cameraPosWorld - fragPosWorld);

	for (int i = 0; i < ubo.numLights; i++) {
		PointLight light = ubo.pointLights[i];
		vec3 direntionToLight = light.position.xyz - fragPosWorld;
		float attenuation = 1.0 / dot(direntionToLight, direntionToLight);
		direntionToLight = normalize(direntionToLight);

		float cosAngIncidence = max(dot(surfaceNormal, direntionToLight), 0);
		vec3 intensity = light.color.xyz * light.color.w * attenuation;

		diffuseLight += intensity * cosAngIncidence;

		// specular lighting

		vec3 halfAngle = normalize(direntionToLight + viewDirection);
		float blinnTerm = dot(surfaceNormal, halfAngle);
		blinnTerm = clamp(blinnTerm, 0, 1);
		blinnTerm = pow(blinnTerm, 512.0); // higher = sharper
		specularLight += intensity * blinnTerm;
	}

    //outColor = vec4(diffuseLight * fragColor + specularLight * fragColor, 1.0);

	// outColor = vec4(fragTexCoord, 0.0, 1.0);
	outColor = texture(texSampler[push.textureIndex], fragTexCoord) * vec4(diffuseLight + specularLight, ubo.ambientLight.w);

}