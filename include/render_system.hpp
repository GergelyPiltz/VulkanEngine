#pragma once

#include "device.hpp"
#include "pipeline.hpp"
#include "game_object.hpp"
#include "camera.hpp"

#include <memory>
#include <vector>

class RenderSystem {
public:
	RenderSystem(Device& device, VkRenderPass renderPass);
	~RenderSystem();

	RenderSystem(const RenderSystem&) = delete;
	RenderSystem& operator=(const RenderSystem&) = delete;
	RenderSystem(RenderSystem&&) = delete;
	RenderSystem& operator=(RenderSystem&&) = delete;

	void renderGameObjects(VkCommandBuffer commandBuffer, std::vector<GameObject>& gameObjects, const Camera& camera);
private:
	void createPipelineLayout();
	void createPipeline(VkRenderPass renderPass);

	Device& device;

	std::unique_ptr<Pipeline> pipeline;
	VkPipelineLayout pipelineLayout;
};