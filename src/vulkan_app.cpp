#include "vulkan_app.hpp"
#include "render_system.hpp"
#include "camera.hpp"
#include "keyboard_movement_controller.hpp"
#include "buffer.hpp"
#include "point_light_system.hpp"
#include "texture_sampler.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>
#include <iostream>
#include <chrono>
#include <memory>

VulkanApp::VulkanApp() {
    globalPool = DescriptorPool::Builder(device)
        .setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
        .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, SwapChain::MAX_FRAMES_IN_FLIGHT)
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
        .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
        .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
        .build();


    Texture texture = Texture(device);
    TextureSampler textureSampler = TextureSampler(device);


    std::vector<VkDescriptorSet> globalSets(SwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < globalSets.size(); i++) {
        auto bufferInfo = uboBuffers[i]->descriptorInfo();

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = texture.textureImageView();
        imageInfo.sampler = textureSampler.textureSampler();


        DescriptorWriter(*globalSetLayout, *globalPool)
            .writeBuffer(0, &bufferInfo)
            .writeImage(1, &imageInfo)
            .build(globalSets[i]);
    }

    RenderSystem renderSystem{ device, renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};

    PointLightSystem pointLightSystem{ device, renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout() };

    Camera camera;
    //camera.setViewDirection(glm::vec3( 0.f ), glm::vec3(.5f, 0.f, 1.f));
    camera.setViewTarget(glm::vec3(-1.f, -2.f, 2.f), glm::vec3(0.f, 0.f, 2.5f));

    auto viewerObject = GameObject::createGameObject();
    viewerObject.transform.translation.y = -1.f;
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
        camera.setPerspectiveProjection(glm::radians(50.f), aspect, .1f, 1000.f);

        if (auto commandBuffer = renderer.beginFrame()) {
            int frameIndex = renderer.getFrameIndex();
            
            FrameInfo frameInfo{
                frameIndex,
                deltaTime,
                commandBuffer,
                camera,
                globalSets[frameIndex],
                gameObjects
            };

            // update
            GlobalUBO ubo{};
            ubo.projection = camera.getProjection();
            ubo.view = camera.getView();
            ubo.inverseView = camera.getInverseView();
            pointLightSystem.update(frameInfo, ubo);
            uboBuffers[frameIndex]->writeToBuffer(&ubo);
            uboBuffers[frameIndex]->flush();

            // render
            renderer.beginSwapChainRenderPass(commandBuffer);

            // Order matters
            renderSystem.renderGameObjects(frameInfo);
            pointLightSystem.render(frameInfo);

            renderer.endSwapChainRenderPass(commandBuffer);
            renderer.endFrame();
        }
    }
    vkDeviceWaitIdle(device.device());
}

void VulkanApp::loadGameObjects() {
    
    std::vector<glm::vec3> lightColors{
        {1.f, .1f, .1f},
        {.1f, .1f, 1.f},
        {.1f, 1.f, .1f},
        {1.f, 1.f, .1f},
        {.1f, 1.f, 1.f},
        {1.f, 1.f, 1.f}
    };

    for (int i = 0; i < lightColors.size(); i++) {
        auto pointLight = GameObject::makePointLight(1.0f, 0.1f, lightColors[i]);

        auto rotateLight = glm::rotate(glm::mat4{ 1.0f }, (i * glm::two_pi<float>()) / lightColors.size(), glm::vec3{0, -1, 0});
        pointLight.transform.translation = rotateLight * glm::vec4{ 0.0f, -1.5f, 2.0f, 1.0f };

        gameObjects.emplace(pointLight.getId(), std::move(pointLight));
    }

    std::shared_ptr<Model> quadModel = Model::createModelFromFile(device, "models/quad.obj");
    auto quad = GameObject::createGameObject();
    quad.model = quadModel;
    quad.transform.translation = { 0.0f, 0.0f, 0.0f };
    quad.transform.scale = { 10.0f, 10.0f, 10.0f };
    gameObjects.emplace(quad.getId(), std::move(quad));

    //std::shared_ptr<Model> flatVaseModel = Model::createModelFromFile(device, "models/flat_vase.obj");
    //auto flatVase = GameObject::createGameObject();
    //flatVase.model = flatVaseModel;
    //flatVase.transform.translation = { -1.0f, 0.0f, 1.5f };
    //flatVase.transform.scale = { 2.0f, 2.0f, 2.0f };
    //flatVase.transform.rotation = { 0.0f, 0.0f, 0.0f };
    //gameObjects.emplace(flatVase.getId(), std::move(flatVase));

    std::shared_ptr<Model> smoothVaseModel = Model::createModelFromFile(device, "models/smooth_vase.obj");
    auto smoothVase = GameObject::createGameObject();
    smoothVase.model = smoothVaseModel;
    smoothVase.transform.translation = { 0.0f, 0.0f, 0.0f };
    smoothVase.transform.scale = { 2.0f, 2.0f, 2.0f };
    smoothVase.transform.rotation = { 0, 0, 0 };
    gameObjects.emplace(smoothVase.getId(), std::move(smoothVase));

    std::shared_ptr<Model> moonModel = Model::createModelFromFile(device, "models/moon.obj");
    auto moon = GameObject::createGameObject();
    moon.model = moonModel;
    moon.transform.translation = { 0.0f, -4.0f, 0.0f };
    moon.transform.scale = { 1.0f, 1.0f, 1.0f };
    gameObjects.emplace(moon.getId(), std::move(moon));

}