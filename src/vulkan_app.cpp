#include "vulkan_app.hpp"
#include "render_system.hpp"
#include "line_system.hpp"
#include "camera.hpp"
#include "keyboard_movement_controller.hpp"
#include "buffer.hpp"
#include "point_light_system.hpp"
#include "texture_sampler.hpp"
#include "physics_system.hpp"

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
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
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

    const char* pathMoon = "textures/moon.jpeg";
    Texture textureMoon = Texture(device, pathMoon);

    const char* pathRotund = "textures/rotund.png";
    Texture textureRotund = Texture(device, pathRotund);

    const char* pathYawn = "textures/yawn.png";
    Texture textureYawn = Texture(device, pathYawn);

    TextureSampler textureSampler = TextureSampler(device);


    std::vector<VkDescriptorSet> globalSets(SwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < globalSets.size(); i++) {
        auto bufferInfo = uboBuffers[i]->descriptorInfo();

        std::vector<VkDescriptorImageInfo> imageInfos(2);
        imageInfos[0] = { textureSampler.textureSampler(), textureRotund.textureImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
        imageInfos[1] = { textureSampler.textureSampler(), textureMoon.textureImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
        
        /*VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = textureMoon.textureImageView();
        imageInfo.sampler = textureSampler.textureSampler();*/


        DescriptorWriter(*globalSetLayout, *globalPool)
            .writeBuffers(0, &bufferInfo, 1)
            .writeImages(1, imageInfos.data(), imageInfos.size())
            .build(globalSets[i]);
    }

    RenderSystem renderSystem{ device, renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout() };
    PointLightSystem pointLightSystem{ device, renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout() };
    LineSystem lineSystem{ device, renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout() };
    PhysicsSystem physicsSystem = PhysicsSystem();

    Camera camera;
    auto viewerObject = GameObject::createGameObject();
    viewerObject.transform = std::make_unique<Transform>();
    viewerObject.transform->translation = {0.0f, -2.0f, -5.0f};
    //camera.setViewDirection(viewerObject.transform.translation, { 0.0f, 0.0f, 0.0f });
    KeyboardMovementController cameraController{};
    
    auto currentTime = std::chrono::high_resolution_clock::now();

    while (!window.shouldClose()) {
        glfwPollEvents();

        auto newTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
        currentTime = newTime;

        cameraController.moveInPlaneXZ(window.getGLFWwindow(), deltaTime, viewerObject);
        camera.setViewYXZ(viewerObject.transform->translation, viewerObject.transform->rotation);

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
            //uboBuffers[frameIndex]->flush();
            physicsSystem.update(frameInfo);

            // render
            renderer.beginSwapChainRenderPass(commandBuffer);

            // Order matters
            renderSystem.renderGameObjects(frameInfo);
            lineSystem.render(frameInfo);
            pointLightSystem.render(frameInfo);

            renderer.endSwapChainRenderPass(commandBuffer);
            renderer.endFrame();
        }
    }
    vkDeviceWaitIdle(device.device());
}

void VulkanApp::loadGameObjects() {
    
    std::vector<glm::vec3> lightColors{
        {1.0f, 0.1f, 0.1f},
        {0.1f, 0.1f, 1.0f},
        {0.1f, 1.0f, 0.1f},
        {1.0f, 1.0f, 0.1f},
        {0.1f, 1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f}
    };

    for (int i = 0; i < lightColors.size(); i++) {
        auto pointLight = GameObject::createGameObject();

        pointLight.transform = std::make_unique<Transform>();
        auto rotateLight = glm::rotate(glm::mat4{ 1.0f }, (i * glm::two_pi<float>()) / lightColors.size(), glm::vec3{0, -1, 0});
        pointLight.transform->translation = rotateLight * glm::vec4{ 0.0f, -1.5f, 2.0f, 1.0f };
        pointLight.transform->scale.x = 0.1f;

        pointLight.pointLight = std::make_unique<PointLight>();
        pointLight.pointLight->color = glm::vec4(lightColors[i], 1.0f);

        gameObjects.emplace(pointLight.getId(), std::move(pointLight));
    }

    //std::shared_ptr<Model> quadModel = Model::createModelFromFile(device, "models/quad.obj");
    //auto quad = GameObject::createGameObject();
    //quad.model = quadModel;
    //quad.transform = std::make_unique<Transform>();
    //quad.transform->translation = { 0.0f, 0.0f, 0.0f };
    //quad.transform->scale = { 10.0f, 10.0f, 10.0f };
    //quad.textureIndex = 0;
    //gameObjects.emplace(quad.getId(), std::move(quad));

    ///////////////////////////////////////////////////////////////////////////
    auto boundingSphere = std::make_unique<RigidBody>();
    std::shared_ptr<Model> sphereModel = Model::sphere(device, 1.0f, 100, 100);
    ///////////////////////////////////////////////////////////////////////////
    auto sphere0 = GameObject::createGameObject();
    sphere0.wireFrame = sphereModel;

    sphere0.transform = std::make_unique<Transform>();
    sphere0.transform->translation = { +5.0f, -3.5f, 0.7f };
    sphere0.transform->scale = { 1.0f, 1.0f, 1.0f };

    sphere0.textureIndex = 1;

    sphere0.boundingSphere = std::make_unique<BoundingSphere>();
    sphere0.boundingSphere->radius = 1.0f;

    sphere0.rigidBody = std::make_unique<RigidBody>();
    sphere0.rigidBody->velocity = { -4.0f, 0.0f, 0.0f };
    sphere0.rigidBody->mass = 1.0f;

    gameObjects.emplace(sphere0.getId(), std::move(sphere0));
    ///////////////////////////////////////////////////////////////////////////
    auto sphere1 = GameObject::createGameObject();
    sphere1.wireFrame = sphereModel;

    sphere1.transform = std::make_unique<Transform>();
    sphere1.transform->translation = { -2.0f, -4.5f, -0.7f };
    sphere1.transform->scale = { 1.0f, 1.0f, 1.0f };

    sphere1.textureIndex = 1;

    sphere1.boundingSphere = std::make_unique<BoundingSphere>();
    sphere1.boundingSphere->radius = 1.0f;

    sphere1.rigidBody = std::make_unique<RigidBody>();
    sphere1.rigidBody->velocity = { +1.0f, 0.0f, 0.0f };
    sphere1.rigidBody->mass = 1.0f;

    gameObjects.emplace(sphere1.getId(), std::move(sphere1));
    ///////////////////////////////////////////////////////////////////////////
 
}