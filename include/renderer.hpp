#pragma once

#include "window.hpp"
#include "device.hpp"
#include "swapchain.hpp"
#include "model.hpp"

#include <memory>
#include <vector>
#include <cassert>

class Renderer {
public:
	Renderer(Window& window, Device& device);
	~Renderer();

	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;
	Renderer(Renderer&&) = delete;
	Renderer& operator=(Renderer&&) = delete;

	VkRenderPass getSwapChainRenderPass() const { return swapChain->getRenderPass(); }
	float getAspectRation() const { return swapChain->extentAspectRatio(); }
	bool isFrameInProgress() const { return isFrameStarted; }

	VkCommandBuffer getCurrentCommandBuffer() const {
		assert(isFrameStarted && "cannot get command buffer when frame is not in progress!");
		return commandBuffers[currentFrameIndex];
	}

	int getFrameIndex() const {
		assert(isFrameStarted && "Cannot get frame index while frame is not in progress!");
		return currentFrameIndex;
	}

	VkCommandBuffer beginFrame();
	void endFrame();
	void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
	void endSwapChainRenderPass(VkCommandBuffer commandBuffer) const;

private:

	void createCommandBuffers();
	void freeCommandBuffers();
	void recreateSwapChain();

	Window& window;
	Device& device;
	std::unique_ptr<SwapChain> swapChain;
	std::vector<VkCommandBuffer> commandBuffers;

	uint32_t currentImageIndex = 0;
	int currentFrameIndex = 0;
	bool isFrameStarted = false;
};