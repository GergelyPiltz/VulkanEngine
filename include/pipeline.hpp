#pragma once
#include "device.hpp"
#include <vector>
#include <string>
#include <cstdint>

struct PipelineConfigInfo {
	PipelineConfigInfo() = default;
	PipelineConfigInfo(const PipelineConfigInfo&) = delete;
	PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

	VkPipelineViewportStateCreateInfo viewportInfo{};
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
	VkPipelineRasterizationStateCreateInfo rasterizationInfo{};
	VkPipelineMultisampleStateCreateInfo multisampleInfo{};
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
	VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
	std::vector<VkDynamicState> dynamicStateEnables{};
	VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
	VkPipelineLayout pipelineLayout = nullptr;
	VkRenderPass renderPass = nullptr;
	uint32_t subpass = 0;
};

class Pipeline {
public:
	Pipeline(
		Device& device,
		const std::string& vertexShaderPath, 
		const std::string& fragmentShaderPath,
		const PipelineConfigInfo& configInfo
	);
	~Pipeline();

	Pipeline(const Pipeline&) = delete;
	Pipeline& operator=(const Pipeline&) = delete;
	Pipeline(Pipeline&&) = delete;
	Pipeline& operator=(Pipeline&&) = delete;

	void bind(VkCommandBuffer commandBuffer) const;

	static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);
private:
	static std::vector<char> readShader(const std::string& path);
	void createGraphicsPipeline(
		const std::string& vertexShaderPath,
		const std::string& fragmentShaderPath,
		const PipelineConfigInfo& configInfo
	);
	void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);
	Device& device; 
	VkPipeline graphicsPipeline;
	VkShaderModule vertShaderModule;
	VkShaderModule fragShaderModule;
};