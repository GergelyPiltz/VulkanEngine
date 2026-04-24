#include "physics_system.hpp"

// std
#include <iostream>
#include <limits>

glm::vec3 gravity = { 0.0f, 1.0f, 0.0f };

//void PhysicsSystem::detect(FrameInfo frameInfo) {
//    for (auto it = frameInfo.gameObjects.cbegin(); it != frameInfo.gameObjects.cend(); it++) {
//        auto& objA = it->second;
//        if (objA.sphereCollider == nullptr) continue;
//
//        for (auto itj = it; itj != frameInfo.gameObjects.cend(); itj++) {
//            if (itj == it) continue;
//            auto& objB = itj->second;
//            if (objB.sphereCollider == nullptr) continue;
//
//            glm::vec3 centerToCenter = (objA.transform->position + objA.sphereCollider->center) - (objB.transform->position + objB.sphereCollider->center);
//            float centerDistance = glm::sqrt(glm::dot(centerToCenter, centerToCenter));
//            float radiusDistance = centerDistance - objA.sphereCollider->radius - objB.sphereCollider->radius;
//
//            if (radiusDistance < std::numeric_limits<float>::epsilon()) {
//                objA.rigidBody->velocity = glm::normalize(centerToCenter) * glm::sqrt(glm::dot(objA.rigidBody->velocity, objA.rigidBody->velocity));
//                objB.rigidBody->velocity = glm::normalize(-centerToCenter) * glm::sqrt(glm::dot(objB.rigidBody->velocity, objB.rigidBody->velocity));
//            }
//
//        }
//    }
//}
//
//float collideSpheres(SphereCollider& first, SphereCollider& second) {
//
//}

void PhysicsSystem::update(FrameInfo frameInfo) {
    for (auto& kvpI : frameInfo.gameObjects) {
        auto& objA = kvpI.second;
        if (objA.rigidBody == nullptr) continue;

        objA.rigidBody->velocity += gravity * objA.rigidBody->mass * frameInfo.frameTime;
        objA.transform->position += objA.rigidBody->velocity * frameInfo.frameTime;
    }

    for (auto it = frameInfo.gameObjects.cbegin(); it != frameInfo.gameObjects.cend(); it++) {
        auto& objA = it->second;
        if (objA.rigidBody == nullptr) continue;
        if (objA.sphereCollider == nullptr) continue;

        for (auto itj = it; itj != frameInfo.gameObjects.cend(); itj++) {
            if (itj == it) continue;
            auto& objB = itj->second;
            if (objB.rigidBody == nullptr) continue;
            if (objB.sphereCollider == nullptr) continue;

            glm::vec3 centerToCenter = (objA.transform->position + objA.sphereCollider->center) - (objB.transform->position + objB.sphereCollider->center);
            float centerDistance = glm::sqrt(glm::dot(centerToCenter, centerToCenter));
            float radiusDistance = centerDistance - objA.sphereCollider->radius - objB.sphereCollider->radius;

            //std::cout << radiusDistance << std::endl;

            if (radiusDistance < std::numeric_limits<float>::epsilon()) {
                objA.rigidBody->velocity = glm::normalize(centerToCenter) * glm::sqrt(glm::dot(objA.rigidBody->velocity, objA.rigidBody->velocity));
                objB.rigidBody->velocity = glm::normalize(-centerToCenter) * glm::sqrt(glm::dot(objB.rigidBody->velocity, objB.rigidBody->velocity));
            }

        }
    }
}