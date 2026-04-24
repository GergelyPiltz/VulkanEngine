#include "line_system.hpp"
#include "global.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>
#include <print>

struct SimplePushConstantData {
    glm::mat4 modelMatrix{ 1.0f };
};

LineSystem::LineSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalDescriptorSetLayout) : device{ device } {
    createPipelineLayout(globalDescriptorSetLayout);
    createPipeline(renderPass);

    sphere = Model::sphere(device, 32, 32);
    cube = Model::cube(device);
}

LineSystem::~LineSystem() {
    vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
}

void LineSystem::createPipelineLayout(VkDescriptorSetLayout globalDescriptorSetLayout) {

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(SimplePushConstantData);

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

void LineSystem::createPipeline(VkRenderPass renderPass) {
    assert(pipelineLayout != nullptr && "cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipelineConfig{};
    Pipeline::defaultPipelineConfigInfo(pipelineConfig);
    //
    pipelineConfig.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    //
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = pipelineLayout;
    pipeline = std::make_unique<Pipeline>(
        device,
        "shaders/line.vert.spv",
        "shaders/line.frag.spv",
        pipelineConfig);
}

void LineSystem::render(FrameInfo& frameInfo) {
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

    for (auto& kvp : frameInfo.gameObjects) {
        auto& obj = kvp.second;
        drawSphere(obj, frameInfo);
        drawCube(obj, frameInfo);
    }
}

void LineSystem::drawSphere(GameObject& object, FrameInfo& frameInfo) {
    if (object.sphereCollider == nullptr) return;

    transform.position = object.transform->position + object.sphereCollider->center;
    transform.scale = glm::vec3{ 1.0f } * object.sphereCollider->radius;
    transform.rotation = object.transform->rotation;

    SimplePushConstantData push{};
    push.modelMatrix = transform.modelMatrix();

    vkCmdPushConstants(
        frameInfo.commandBuffer,
        pipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        0,
        sizeof(SimplePushConstantData),
        &push);
    sphere->bind(frameInfo.commandBuffer);
    sphere->draw(frameInfo.commandBuffer);
}

void LineSystem::drawCube(GameObject& object, FrameInfo& frameInfo) {
    if (object.boxCollider == nullptr) return;

    auto& min = object.boxCollider->minExtent;
    auto& max = object.boxCollider->maxExtent;

    assert((min.x < max.x) && (min.y < max.y) && (min.z < max.z) && "incorrect setup of min/max extent of box collider!");

    glm::vec3 diagonal = object.boxCollider->maxExtent - object.boxCollider->minExtent;
    transform.scale = diagonal;
    transform.position = object.transform->position + (0.5f * (object.boxCollider->maxExtent + object.boxCollider->minExtent));
    //transform.rotation = object.transform->rotation;

    SimplePushConstantData push{};
    push.modelMatrix = transform.modelMatrix();

    vkCmdPushConstants(
        frameInfo.commandBuffer,
        pipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        0,
        sizeof(SimplePushConstantData),
        &push);
    cube->bind(frameInfo.commandBuffer);
    cube->draw(frameInfo.commandBuffer);
}
