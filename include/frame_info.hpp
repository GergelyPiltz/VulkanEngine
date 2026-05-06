#pragma once
#include "camera.hpp"
#include "game_object.hpp"

// libs
#include <vulkan/vulkan.h>

struct FrameInfo {
	int frameIndex;
	float deltaTime;
	VkCommandBuffer commandBuffer;
	Camera& camera;
	VkDescriptorSet globalDescriptorSet;
	GameObject::Map& gameObjects;
};

struct PointLightUBO {
	glm::vec4 position{};
	glm::vec4 color{};
};

#define MAX_LIGHTS 10

struct GlobalUBO {
	glm::mat4 projection{ 1.f };
	glm::mat4 view{ 1.f };
	glm::mat4 inverseView{ 1.f };
	// alignas(16) glm::vec3 lightDirection = glm::normalize(glm::vec3(1.f, -3.f, -1.f));
	glm::vec4 ambientColor{ 1.f, 1.f, 1.f, .02f };
	PointLightUBO pointLights[MAX_LIGHTS];
	int numLights;
};