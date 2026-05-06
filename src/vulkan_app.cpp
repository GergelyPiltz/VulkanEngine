#include "vulkan_app.hpp"
#include "render_system.hpp"
#include "line_system.hpp"
#include "camera.hpp"
#include "keyboard_movement_controller.hpp"
#include "buffer.hpp"
#include "point_light_system.hpp"
#include "texture_sampler.hpp"
#include "physics_system.hpp"
#include "noise.hpp"
#include "scalar_field.hpp"
#include "terrain_generator.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>
#include <iostream>
#include <chrono>
#include <memory>
#include <print>

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

    const char* pathGrass = "textures/grass.png";
    Texture textureGrass = Texture(device, pathGrass);

    const char* pathRotund = "textures/rotund.png";
    Texture textureRotund = Texture(device, pathRotund);

    const char* pathYawn = "textures/yawn.png";
    Texture textureYawn = Texture(device, pathYawn);

    TextureSampler textureSampler = TextureSampler(device);


    std::vector<VkDescriptorSet> globalSets(SwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < globalSets.size(); i++) {
        auto bufferInfo = uboBuffers[i]->descriptorInfo();

        std::vector<VkDescriptorImageInfo> imageInfos(2);
        imageInfos[0] = { textureSampler.textureSampler(), textureGrass.textureImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
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
    viewerObject.transform->position = {0.0f, 0.0f, -15.0f};
    //camera.setViewDirection(viewerObject.transform.translation, { 0.0f, 0.0f, 0.0f });
    KeyboardMovementController cameraController{};
    
    auto currentTime = std::chrono::high_resolution_clock::now();

    while (!window.shouldClose()) {
        glfwPollEvents();

        auto newTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
        currentTime = newTime;

        cameraController.moveInPlaneXZ(window.getGLFWwindow(), deltaTime, viewerObject);
        camera.setViewYXZ(viewerObject.transform->position, viewerObject.transform->rotation);

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

            if (glfwGetKey(window.getGLFWwindow(), GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS) frameInfo.deltaTime = -frameInfo.deltaTime;
            if (glfwGetKey(window.getGLFWwindow(), GLFW_KEY_SPACE) == GLFW_PRESS) frameInfo.deltaTime = 0.0f;

            // update
            GlobalUBO ubo{};
            ubo.projection = camera.getProjection();
            ubo.view = camera.getView();
            ubo.inverseView = camera.getInverseView();
            ubo.ambientColor = { 1.0f, 1.0f, 1.0f, 0.3f };

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
    
    int axis = 32;
    int axisPlusOne = axis + 1;

    Noise noise{ 0 };
    double** noiseArray = noise.Generate(axisPlusOne, axisPlusOne);

    ScalarField scalarField{ noiseArray, axisPlusOne, axisPlusOne, axisPlusOne };
     
    TerrainGenerator terrainGenerator{ scalarField.get(), axis };
    terrainGenerator.createMeshData();
    std::vector<glm::vec3> raw = terrainGenerator.getMeshData();
     
    std::vector<Model::Vertex> vertices(raw.size());
    std::vector<glm::vec3> normals(raw.size() / 3);
     
    for (int i = 0; i < raw.size(); i+=3) {
        glm::vec3 A = raw[i + 1] - raw[i];
        glm::vec3 B = raw[i + 2] - raw[i];
        glm::vec3 normal = glm::normalize(glm::cross(A, B));
        //std::println("({0}, {1}, {2})", normal.x, normal.y, normal.z);
        normals.push_back(normal);
    }
     
    for(int i = 0; i < raw.size(); i++) {
        auto n = normals[(int)(i / 3)];
        //std::println("({0}, {1}, {2})", raw[i].x, raw[i].y, raw[i].z);
        vertices.push_back(Model::Vertex{ raw[i], {0.0f, -1.0f, 0.0f}, {raw[i].x, raw[i].z} });
    }
     
    Model::Builder builder;
    builder.vertices = vertices;
     
    std::shared_ptr<Model> terrainModel = std::make_shared<Model>(device, builder);    
    /////////////////////////////////////////////////////////////////////////////
    auto terrain = GameObject::createGameObject();
    terrain.model = terrainModel;
     
    terrain.transform = std::make_unique<Transform>();
    terrain.transform->position = { 0.0f, 0.0f, 0.0f };
    terrain.transform->scale = { 1.0f, 1.0f, 1.0f };
     
    terrain.textureIndex = 0;
     
    terrain.meshCollider = std::make_unique<MeshCollider>();
    terrain.meshCollider->vertices = raw;
     
    gameObjects.emplace(terrain.getId(), std::move(terrain));
    /////////////////////////////////////////////////////////////////////////////
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
        pointLight.transform->position = rotateLight * glm::vec4{ 0.0f, 10.0f, 2.0f, 1.0f };
        pointLight.transform->scale.x = 0.1f;
     
        pointLight.pointLight = std::make_unique<PointLight>();
        pointLight.pointLight->color = glm::vec4(lightColors[i], 2.0f);
     
        gameObjects.emplace(pointLight.getId(), std::move(pointLight));
    }
    ///////////////////////////////////////////////////////////////////////////

    std::shared_ptr<Model> quadModel = Model::createModelFromFile(device, "models/quad.obj");
    auto quad = GameObject::createGameObject();
    quad.model = quadModel;
    quad.transform = std::make_unique<Transform>();
    quad.transform->position = { 0.0f, 11.0f, 0.0f };
    quad.transform->scale = { 10.0f, 10.0f, 10.0f };
    quad.textureIndex = 1;
    gameObjects.emplace(quad.getId(), std::move(quad));

    ///////////////////////////////////////////////////////////////////////////
    for (int x = -1; x < 2; x++)
    for (int y = -1; y < 2; y++)
    for (int z = -1; z < 2; z++)
    {
        auto sphere = GameObject::createGameObject();

        sphere.transform = std::make_unique<Transform>();
        sphere.transform->position = {x * 2, y * 2, z * 2};
        sphere.transform->scale = { 1.0f, 1.0f, 1.0f };

        sphere.sphereCollider = std::make_unique<SphereCollider>();
        sphere.sphereCollider->radius = 0.5f;

        sphere.rigidBody = std::make_unique<RigidBody>();
        sphere.rigidBody->velocity = { rand() % 10 + 1, rand() % 10 + 1, rand() % 10 + 1 };
        sphere.rigidBody->mass = 1.0f;

        gameObjects.emplace(sphere.getId(), std::move(sphere));
    }
    ///////////////////////////////////////////////////////////////////////////
    auto cube = GameObject::createGameObject();
    
    cube.transform = std::make_unique<Transform>();
    cube.transform->position = { 0.0f, 0.0f, 0.0f };
    cube.transform->scale = { 1.0f, 1.0f, 1.0f };

    cube.boxCollider = std::make_unique<BoxCollider>();
    cube.boxCollider->minExtent = { -5.0f, -5.0f, -5.0f };
    cube.boxCollider->maxExtent = { 5.0f, 5.0f, 5.0f };
    
    cube.rigidBody = std::make_unique<RigidBody>();
    cube.rigidBody->velocity = { 0.0f, 0.0f, 0.0f };
    cube.rigidBody->mass = 0.0f;
    
    gameObjects.emplace(cube.getId(), std::move(cube));
    ///////////////////////////////////////////////////////////////////
    auto ORIGIN = GameObject::createGameObject();

    ORIGIN.transform = std::make_unique<Transform>();
    ORIGIN.transform->position = { 0.0f, 0.0f, 0.0f };
    ORIGIN.transform->scale.x = 0.1f;

    ORIGIN.pointLight = std::make_unique<PointLight>();
    ORIGIN.pointLight->color = glm::vec4( 1.0f );

    gameObjects.emplace(ORIGIN.getId(), std::move(ORIGIN));
    ///////////////////////////////////////////////////////////////////
    auto vase = GameObject::createGameObject();
    std::shared_ptr<Model> vaseModel = Model::createModelFromFile(device, "models/smooth_vase.obj");
    vase.model = vaseModel;

    vase.transform = std::make_unique<Transform>();
    vase.transform->position = { 0.0f, 11.0f, 0.0f };
    vase.transform->scale = { 5.0f, 5.0f, 5.0f };

    vase.textureIndex = 1;

    gameObjects.emplace(vase.getId(), std::move(vase));
    ///////////////////////////////////////////////////////////////////
    // auto chunkBounding = GameObject::createGameObject();
    // 
    // chunkBounding.transform = std::make_unique<Transform>();
    // chunkBounding.transform->position = { 0.0f, 0.0f, 0.0f };
    // chunkBounding.transform->scale = { 1.0f, 1.0f, 1.0f };
    // 
    // chunkBounding.boxCollider = std::make_unique<BoxCollider>();
    // chunkBounding.boxCollider->minExtent = { -0.0f, -0.0f , -0.0f };
    // chunkBounding.boxCollider->maxExtent = { 32.0f, 32.0f , 32.0f };
    // 
    // gameObjects.emplace(chunkBounding.getId(), std::move(chunkBounding));
 
}