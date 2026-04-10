#pragma once

#include "global.hpp"
#include "window.hpp"
#include "device.hpp"
#include "game_object.hpp"
#include "renderer.hpp"
#include "render_system.hpp"
#include "descriptors.hpp"

#include <memory>
#include <vector>

class VulkanApp {
public:
	VulkanApp();
	~VulkanApp();

	VulkanApp(const VulkanApp&) = delete;
	VulkanApp& operator=(const VulkanApp&) = delete;
	VulkanApp(VulkanApp&&) = delete;
	VulkanApp& operator=(VulkanApp&&) = delete;

	void run();

private:
	void loadGameObjects();

	Window window{ WIDTH, HEIGHT, TITLE };
	Device device{ window };
	Renderer renderer{ window, device };

	// note: must be declared after the Device
	std::unique_ptr<DescriptorPool> globalPool{};
	GameObject::Map gameObjects;
};