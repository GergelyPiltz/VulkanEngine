#include "physics_system.hpp"
#include "utils.hpp"

// std
#include <limits>
#include <print>

/*

between two colliders only a single collision point is present???
right?
multiple colliders on a single object can collide with an other collider on a single object ????


detect
    shpere - sphere -> log collision point
    shpere - AABB -> log collision point
    shpere - plane -> log collision point
    shpere - mesh -> log collision point
    shpere -  -> log collision point
    shpere - sphere -> log collision point
    shpere - sphere -> log collision point
note collision 
        (collider detection, but multiple collisions are possible on single object???)
        (distribute a single amount of force between collisions???)
    object this  - info (object other, point, vector of force)
    object other - info (object this, point, vector of force)
        (emplace both so another set of calculatons are not required in reverse)
loop collision list
    get collision point
        resolve force from point
            get other mass, get our mass, get our center of mass
                execute force (executing a combined single force can resolve multiple collisions)


*/

//void PhysicsSystem::detect(FrameInfo& frameInfo) {
//    for (auto it = frameInfo.gameObjects.cbegin(); it != frameInfo.gameObjects.cend(); it++) {
//        auto& objA = it->second;
//
//        for (auto itj = it; itj != frameInfo.gameObjects.cend(); itj++) {
//
//        }
//    }
//}

static void keepWithinBoundingVolume(GameObject& ball) {
    const float size = 5.0f;

    if (!ball.rigidBody) return;
    if (!ball.sphereCollider) return;

    glm::vec3 pos = ball.transform->position + ball.sphereCollider->center;

    if (pos.x > +size) ball.rigidBody->velocity.x = -glm::abs(ball.rigidBody->velocity.x);
    if (pos.x < -size) ball.rigidBody->velocity.x = +glm::abs(ball.rigidBody->velocity.x);

    if (pos.y > +size) ball.rigidBody->velocity.y = -glm::abs(ball.rigidBody->velocity.y);
    if (pos.y < -size) ball.rigidBody->velocity.y = +glm::abs(ball.rigidBody->velocity.y);

    if (pos.z > +size) ball.rigidBody->velocity.z = -glm::abs(ball.rigidBody->velocity.z);
    if (pos.z < -size) ball.rigidBody->velocity.z = +glm::abs(ball.rigidBody->velocity.z);
}

void PhysicsSystem::resolveCollisions(FrameInfo& frameInfo) {
    for (auto it = collisions.cbegin(); it != collisions.cend(); it++)
    {
        auto& ffirst = frameInfo.gameObjects.at(it->first);
        auto& second = frameInfo.gameObjects.at(it->second);

        // back up to before contact
        ffirst.transform->position -= ffirst.rigidBody->velocity * frameInfo.deltaTime;
        second.transform->position -= second.rigidBody->velocity * frameInfo.deltaTime;

        float energyA = glm::length(ffirst.rigidBody->velocity * ffirst.rigidBody->mass);
        float energyB = glm::length(second.rigidBody->velocity * second.rigidBody->mass);
        std::println("-----------------------COLLISION-----------------------");
        std::println("BEFORE-------------------------------------------------");
        std::println("Velocity A: {}", glm::length(ffirst.rigidBody->velocity));
        std::println("Energy A:   {}", energyA);
        std::println("Velocity B: {}", glm::length(second.rigidBody->velocity));
        std::println("Energy B:   {}", energyB);
        std::println("Total energy:      {}", energyA + energyB);

        glm::vec3 v1 = ffirst.rigidBody->velocity;
        glm::vec3 v2 = second.rigidBody->velocity;

        float m1 = ffirst.rigidBody->mass;
        float m2 = second.rigidBody->mass;

        glm::vec3 c1 = ffirst.transform->position + ffirst.sphereCollider->center;
        glm::vec3 c2 = second.transform->position + second.sphereCollider->center;

        //glm::vec3 cd = (ffirst.transform->position + ffirst.sphereCollider->center) - (second.transform->position + ffirst.sphereCollider->center);
        //glm::vec3 cn = glm::normalize(cd);

        // float angle = glm::dot(safeNormalize(v1), safeNormalize(-cd));
        // std::println("ANGLE--------------------------------------------------");
        // std::println("angle: {}", angle);

        ffirst.rigidBody->velocity = v1 - ((2 * m2) / (m1 + m2)) * (glm::dot(v1 - v2, c1 - c2) / glm::length2(c1 - c2)) * (c1 - c2);
        second.rigidBody->velocity = v2 - ((2 * m1) / (m1 + m2)) * (glm::dot(v2 - v1, c2 - c1) / glm::length2(c2 - c1)) * (c2 - c1);


        //first_.rigidBody->velocity = ((v1 * m1) + (collisionNormal * m2)) / m1;
        //second.rigidBody->velocity = ((v2 * m2) + (-collisionNormal * m1)) / m2;

        energyA = glm::length(ffirst.rigidBody->velocity * ffirst.rigidBody->mass);
        energyB = glm::length(second.rigidBody->velocity * second.rigidBody->mass);
        std::println("AFTER--------------------------------------------------");
        std::println("Velocity A: {}", glm::length(ffirst.rigidBody->velocity));
        std::println("Energy A:   {}", energyA);
        std::println("Velocity B: {}", glm::length(second.rigidBody->velocity));
        std::println("Energy B:   {}", energyB);
        std::println("Total energy:      {}", energyA + energyB);


        

        

        //glm::vec3 collisionDirection = (first_.transform->position + first_.sphereCollider->center) - (second.transform->position + first_.sphereCollider->center);
        //
        //float Va = glm::length(first_.rigidBody->velocity);
        //float Vb = glm::length(second.rigidBody->velocity);
        //
        //float Ma = first_.rigidBody->mass;
        //float Mb = second.rigidBody->mass;
        //
        //float Va2 = ((Ma - Mb) / (Ma + Mb) * Va) + ((2.0f * Mb) / (Ma + Mb) * Vb);
        //float Vb2 = ((2.0f * Ma) / (Ma + Mb) * Va) + ((Mb - Ma) / (Ma + Mb) * Vb);
        //
        //glm::vec3 Da = safeNormalize(first_.rigidBody->velocity) + safeNormalize(collisionDirection);
        //glm::vec3 Db = safeNormalize(second.rigidBody->velocity) + safeNormalize(-collisionDirection);
        //
        //first_.rigidBody->velocity = (first_.rigidBody->velocity * first_.rigidBody->mass) + (glm::normalize(collisionDirection) + second.rigidBody->mass); //glm::normalize(Da) * glm::abs(Va2);
        //second.rigidBody->velocity = (second.rigidBody->velocity * second.rigidBody->mass) + (glm::normalize(-collisionDirection) + first_.rigidBody->mass); //glm::normalize(Db) * glm::abs(Vb2);

        
        
    }
}



bool collideSpheres(const GameObject& first, const GameObject& second) {
    if (!first.sphereCollider) return false;
    if (!second.sphereCollider) return false;

    glm::vec3 centerToCenter = (first.transform->position + first.sphereCollider->center) - (second.transform->position + second.sphereCollider->center);
    float centerDistance = glm::sqrt(glm::dot(centerToCenter, centerToCenter));
    float radiusDistance = centerDistance - first.sphereCollider->radius - second.sphereCollider->radius;

    if (radiusDistance < std::numeric_limits<float>::epsilon()) return true;
    return false;
}

void updateAllRigidBodies(FrameInfo& frameInfo) {
    for (auto& kvp : frameInfo.gameObjects) {
        auto& objA = kvp.second;
        if (!objA.rigidBody) continue;

        keepWithinBoundingVolume(objA);
        // objA.rigidBody->velocity += gravity * objA.rigidBody->mass * frameInfo.frameTime;
        objA.transform->position += objA.rigidBody->velocity * frameInfo.deltaTime;
    }
}

void PhysicsSystem::update(FrameInfo& frameInfo) {
    collisions.clear();
    updateAllRigidBodies(frameInfo);

    for (auto it = frameInfo.gameObjects.cbegin(); it != frameInfo.gameObjects.cend(); it++) {
        auto& objA = it->second;

        for (auto itj = it; itj != frameInfo.gameObjects.cend(); itj++) {
            if (itj == it) continue;
            auto& objB = itj->second;

            
            if (collideSpheres(objA, objB)) {
                

                collisions.emplace(objA.getId(), objB.getId());

            }

        }
    }
    resolveCollisions(frameInfo);
}