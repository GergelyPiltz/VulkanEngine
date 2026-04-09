#include "window.hpp"

Window::Window(int width, int height, const char* title) : width { width }, height { height } {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, frameBufferResizedCallback);
}

Window::~Window() {
	glfwDestroyWindow(window);
	glfwTerminate();
}

void Window::frameBufferResizedCallback(GLFWwindow* window, int width, int height) {
    auto resizedWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    resizedWindow->frameBufferResized = true;
    resizedWindow->width = width;
    resizedWindow->height = height;
}