#include "point_light_system.hpp"
#include "global.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <stdexcept>
#include <array>
#include <iostream>
#include <map>

struct PointLightPushConstants {
    glm::vec4 position{};
    glm::vec4 color{};
    float radius;
};

PointLightSystem::PointLightSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalDescriptorSetLayout) : device{ device } {
    createPipelineLayout(globalDescriptorSetLayout);
    createPipeline(renderPass);
}

PointLightSystem::~PointLightSystem() {
    vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
}

void PointLightSystem::createPipelineLayout(VkDescriptorSetLayout globalDescriptorSetLayout) {

     VkPushConstantRange pushConstantRange{};
     pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
     pushConstantRange.offset = 0;
     pushConstantRange.size = sizeof(PointLightPushConstants);

    std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalDescriptorSetLayout };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    if (vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}

void PointLightSystem::createPipeline(VkRenderPass renderPass) {
    assert(pipelineLayout != nullptr && "cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipelineConfig{};
    Pipeline::defaultPipelineConfigInfo(pipelineConfig);
    Pipeline::enableAlphaBlending(pipelineConfig);
    pipelineConfig.bindingDescriptions.clear();
    pipelineConfig.attributeDescriptions.clear();
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = pipelineLayout;
    pipeline = std::make_unique<Pipeline>(
        device,
        "shaders/point_light.vert.spv",
        "shaders/point_light.frag.spv",
        pipelineConfig);
}

void PointLightSystem::update(FrameInfo& frameInfo, GlobalUBO& ubo) {

    auto rotateLight = glm::rotate(glm::mat4{ 1.0f }, frameInfo.frameTime / 10.0f, glm::vec3{ 0, -1, 0 });

    int lightIndex = 0;

    for (auto& kv : frameInfo.gameObjects) {
        auto& obj = kv.second;
        if (obj.pointLight == nullptr) continue;
        obj.transform->translation = glm::vec3(rotateLight * glm::vec4(obj.transform->translation, 1.0f));

        ubo.pointLights[lightIndex].position = glm::vec4(obj.transform->translation, 1.0f);
        ubo.pointLights[lightIndex].color = obj.pointLight->color;

        lightIndex += 1;
    }
    ubo.numLights = lightIndex;
}

void PointLightSystem::render(FrameInfo& frameInfo) {
    // sort lights
    std::map<float, GameObject::id_t> sorted;
    for (auto& kv : frameInfo.gameObjects) {
        auto& obj = kv.second;
        if (obj.pointLight == nullptr) continue;

        // calculate distance
        auto offset = frameInfo.camera.getPosition() - obj.transform->translation;
        float disSquared = glm::dot(offset, offset);
        sorted[disSquared] = obj.getId();
    }

    pipeline->bind(frameInfo.commandBuffer);

    vkCmdBindDescriptorSets(
        frameInfo.commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout,
        0,
        1,
        &frameInfo.globalDescriptorSet,
        0,
        nullptr
    );

    // iterate in reverse order
    for (auto it = sorted.rbegin(); it != sorted.rend(); ++it) {
        
        auto& obj = frameInfo.gameObjects.at(it->second);

        PointLightPushConstants push{};

        push.position = glm::vec4(obj.transform->translation, 1.0f);
        push.color = obj.pointLight->color;
        push.radius = obj.transform->scale.x;

        vkCmdPushConstants(
            frameInfo.commandBuffer,
            pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(PointLightPushConstants),
            &push
        );

        vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
    }

}