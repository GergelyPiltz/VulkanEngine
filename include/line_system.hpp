#pragma once

#include "device.hpp"
#include "pipeline.hpp"
#include "game_object.hpp"
#include "camera.hpp"
#include "frame_info.hpp"

#include <memory>
#include <vector>

class LineSystem {
public:
	LineSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalDescriptorSetLayout);
	~LineSystem();

	LineSystem(const LineSystem&) = delete;
	LineSystem& operator=(const LineSystem&) = delete;
	LineSystem(LineSystem&&) = delete;
	LineSystem& operator=(LineSystem&&) = delete;

	void render(FrameInfo& frameInfo);
private:
	void createPipelineLayout(VkDescriptorSetLayout globalDescriptorSetLayout);
	void createPipeline(VkRenderPass renderPass);

	Device& device;

	std::unique_ptr<Pipeline> pipeline;
	VkPipelineLayout pipelineLayout;
};