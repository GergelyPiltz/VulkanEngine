#pragma once
#include <stdexcept>
#include <cstdint>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class Window {
public:
	Window(int width, int height, const char* title);
	~Window();

	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;
	Window(Window&&) = delete;
	Window& operator=(Window&&) = delete;

	bool shouldClose() const { return glfwWindowShouldClose(window); }
	void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) const { 
		if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create window surface!");
		}
	}
	VkExtent2D getExtent() const { return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) }; }
	bool wasWindowResized() const { return frameBufferResized; }
	void resetWindowResizedFlag() { frameBufferResized = false; }
	GLFWwindow* getGLFWwindow() const { return window; }

private:
	static void frameBufferResizedCallback(GLFWwindow* window, int width, int height);
	GLFWwindow* window;
	int width;
	int height;
	bool frameBufferResized = false;
};