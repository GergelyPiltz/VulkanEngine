#include "physics_system.hpp"

// std
#include <iostream>
#include <limits>

glm::vec3 gravity = { 0.0f, 1.0f, 0.0f };

void PhysicsSystem::update(FrameInfo frameInfo) {
    for (auto& kvpI : frameInfo.gameObjects) {
        auto& objA = kvpI.second;
        if (objA.rigidBody == nullptr) continue;

        objA.rigidBody->velocity += gravity * objA.rigidBody->mass * frameInfo.frameTime;
        objA.transform->translation += objA.rigidBody->velocity * frameInfo.frameTime;
    }

    for (auto it = frameInfo.gameObjects.cbegin(); it != frameInfo.gameObjects.cend(); it++) {
        auto& objA = it->second;
        if (objA.rigidBody == nullptr) continue;
        if (objA.boundingSphere == nullptr) continue;

        for (auto itj = it; itj != frameInfo.gameObjects.cend(); itj++) {
            if (itj == it) continue;
            auto& objB = itj->second;
            if (objB.rigidBody == nullptr) continue;
            if (objB.boundingSphere == nullptr) continue;

            glm::vec3 centerToCenter = objA.transform->translation - objB.transform->translation;
            float centerDistance = glm::sqrt(glm::dot(centerToCenter, centerToCenter));
            float radiusDistance = centerDistance - objA.boundingSphere->radius - objB.boundingSphere->radius;

            std::cout << radiusDistance << std::endl;

            if (radiusDistance < std::numeric_limits<float>::epsilon()) {
                objA.rigidBody->velocity = glm::normalize(centerToCenter) * glm::sqrt(glm::dot(objA.rigidBody->velocity, objA.rigidBody->velocity));
                objB.rigidBody->velocity = glm::normalize(-centerToCenter) * glm::sqrt(glm::dot(objB.rigidBody->velocity, objB.rigidBody->velocity));
            }

        }
    }
}