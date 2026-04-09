#include "vulkan_app.hpp"
#include "render_system.hpp"
#include "camera.hpp"
#include "keyboard_movement_controller.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>
#include <iostream>
#include <chrono>

VulkanApp::VulkanApp() {
    loadGameObjects();
}

VulkanApp::~VulkanApp() {}

void VulkanApp::run() {
    //std::println("Max push constants size: {0}", std::to_string(device.properties.limits.maxPushConstantsSize));

    RenderSystem renderSystem{ device, renderer.getSwapChainRenderPass() };

    Camera camera;
    //camera.setViewDirection(glm::vec3( 0.f ), glm::vec3(.5f, 0.f, 1.f));
    camera.setViewTarget(glm::vec3(-1.f, -2.f, 2.f), glm::vec3(0.f, 0.f, 2.5f));

    auto viewerObject = GameObject::createGameObject();
    KeyboardMovementController cameraController{};
    
    auto currentTime = std::chrono::high_resolution_clock::now();

    while (!window.shouldClose()) {
        glfwPollEvents();

        auto newTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
        currentTime = newTime;

        cameraController.moveInPlaneXZ(window.getGLFWwindow(), deltaTime, viewerObject);
        camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

        // std::cout << "deltaTime: " << deltaTime << std::endl;

        float aspect = renderer.getAspectRation();
        //camera.setOrthographicProjection(-aspect, aspect, -10, 10, -10, 10);
        camera.setPerspectiveProjection(glm::radians(50.f), aspect, .1f, 10.f);

        if (auto commandBuffer = renderer.beginFrame()) {
            renderer.beginSwapChainRenderPass(commandBuffer);
            renderSystem.renderGameObjects(commandBuffer, gameObjects, camera);
            renderer.endSwapChainRenderPass(commandBuffer);
            renderer.endFrame();
        }
    }
    vkDeviceWaitIdle(device.device());
}

void VulkanApp::loadGameObjects() {
    for (size_t x = 0; x < 10; x++)
        for (size_t z = 0; z < 10; z++)
        {
            std::shared_ptr<Model> model = Model::createModelFromFile(device, "models/colored_cube.obj");
            auto gameObj = GameObject::createGameObject();
            gameObj.model = model;
            gameObj.transform.translation = { x * 2, 1, z * 2 };
            gameObj.transform.scale = { .2f, .2f, .2f };
            gameObjects.push_back(std::move(gameObj));
        }

    std::shared_ptr<Model> model = Model::createModelFromFile(device, "models/flat_vase.obj");
    auto gameObj = GameObject::createGameObject();
    gameObj.model = model;
    gameObj.transform.translation = { -1.0f, .5f, 1.5f };
    gameObj.transform.scale = { 1.5f, 1.5f, 1.5f };
    gameObjects.push_back(std::move(gameObj));

    std::shared_ptr<Model> model1 = Model::createModelFromFile(device, "models/smooth_vase.obj");
    auto gameObj1 = GameObject::createGameObject();
    gameObj1.model = model1;
    gameObj1.transform.translation = { 1.0f, .5f, 1.5f };
    gameObj1.transform.scale = { 1.5f, 1.5f, 1.5f };
    gameObjects.push_back(std::move(gameObj1));

}