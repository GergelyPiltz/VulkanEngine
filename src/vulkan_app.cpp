#include "vulkan_app.hpp"
#include "render_system.hpp"
#include "camera.hpp"
#include "keyboard_movement_controller.hpp"
#include "buffer.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>
#include <iostream>
#include <chrono>
#include <memory>

struct GlobalUBO {
    alignas(16) glm::mat4 projectionView{ 1.f };
    alignas(16) glm::vec3 lightDirection = glm::normalize(glm::vec3(1.f, -3.f, -1.f));
};

VulkanApp::VulkanApp() {
    globalPool = DescriptorPool::Builder(device)
        .setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
        .build();
    loadGameObjects();
}

VulkanApp::~VulkanApp() {}

void VulkanApp::run() {
    std::vector<std::unique_ptr<Buffer>> uboBuffers(SwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
        uboBuffers[i] = std::make_unique<Buffer>(
            device,
            sizeof(GlobalUBO),
            1, // only one instance
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            device.properties.limits.minUniformBufferOffsetAlignment
        );
        uboBuffers[i]->map();
    }

    // doesnt have correct alignment, could fix manually
    // Buffer globalUBOBuffer = Buffer(
    //     device,
    //     sizeof(GlobalUBO),
    //     SwapChain::MAX_FRAMES_IN_FLIGHT,
    //     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
    //     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
    //     device.properties.limits.minUniformBufferOffsetAlignment
    // );
    // globalUBOBuffer.map();

    // std::println("Max push constants size: {0}", std::to_string(device.properties.limits.maxPushConstantsSize));

    auto globalSetLayout = DescriptorSetLayout::Builder(device)
        .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
        .build();

    std::vector<VkDescriptorSet> globalSets(SwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < globalSets.size(); i++) {
        auto bufferInfo = uboBuffers[i]->descriptorInfo();
        DescriptorWriter(*globalSetLayout, *globalPool)
            .writeBuffer(0, &bufferInfo)
            .build(globalSets[i]);
    }
    RenderSystem renderSystem{ device, renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};

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
            int frameIndex = renderer.getFrameIndex();
            
            FrameInfo frameInfo{
                frameIndex,
                deltaTime,
                commandBuffer,
                camera,
                globalSets[frameIndex]
            };

            // update
            GlobalUBO ubo{};
            ubo.projectionView = camera.getProjection() * camera.getView();
            uboBuffers[frameIndex]->writeToBuffer(&ubo);
            uboBuffers[frameIndex]->flush();

            // render
            renderer.beginSwapChainRenderPass(commandBuffer);
            renderSystem.renderGameObjects(frameInfo, gameObjects);
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