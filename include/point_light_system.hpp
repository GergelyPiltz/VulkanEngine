#pragma once
#include "device.hpp"
#include "pipeline.hpp"
#include "game_object.hpp"
#include "camera.hpp"
#include "frame_info.hpp"

// std
#include <memory>
#include <vector>

class PointLightSystem {
public:
	PointLightSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalDescriptorSetLayout);
	~PointLightSystem();

	PointLightSystem(const PointLightSystem&) = delete;
	PointLightSystem& operator=(const PointLightSystem&) = delete;
	PointLightSystem(PointLightSystem&&) = delete;
	PointLightSystem& operator=(PointLightSystem&&) = delete;

	void update(FrameInfo& frameInfo, GlobalUBO& ubo);
	void render(FrameInfo& frameInfo);
private:
	void createPipelineLayout(VkDescriptorSetLayout globalDescriptorSetLayout);
	void createPipeline(VkRenderPass renderPass);

	Device& device;

	std::unique_ptr<Pipeline> pipeline;
	VkPipelineLayout pipelineLayout;
};