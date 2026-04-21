#include "game_object.hpp"

glm::mat4 Transform::modelMatrix() const {
    //const float s1 = glm::sin(rotation.x);
    //const float c1 = glm::cos(rotation.x);
    //const float s2 = glm::sin(rotation.y);
    //const float c2 = glm::cos(rotation.y);
    //const float s3 = glm::sin(rotation.z);
    //const float c3 = glm::cos(rotation.z);

    //return glm::mat4{
    //    {
    //        scale.x * (c2 * c3),
    //        scale.x * (c1 * s3 + c3 * s1 * s2),
    //        scale.x * (s1 * s3 - c1 * c3 * s2),
    //        0.0f,
    //    },
    //    {
    //        scale.y * (-c2 * s3),
    //        scale.y * (c1 * c3 - s1 * s2 * s3),
    //        scale.y * (c3 * s1 + c1 * s2 * s3),
    //        0.0f,
    //    },
    //    {
    //        scale.z * (s2),
    //        scale.z * (-c2 * s1),
    //        scale.z * (c1 * c2),
    //        0.0f,
    //    },
    //    {translation.x, translation.y, translation.z, 1.0f} 
    //};

    const float c3 = glm::cos(rotation.z);
    const float s3 = glm::sin(rotation.z);
    const float c2 = glm::cos(rotation.x);
    const float s2 = glm::sin(rotation.x);
    const float c1 = glm::cos(rotation.y);
    const float s1 = glm::sin(rotation.y);

    return glm::mat4{
        {
            scale.x * (c1 * c3 + s1 * s2 * s3),
            scale.x * (c2 * s3),
            scale.x * (c1 * s2 * s3 - c3 * s1),
            0.0f,
        },
        {
            scale.y * (c3 * s1 * s2 - c1 * s3),
            scale.y * (c2 * c3),
            scale.y * (c1 * c3 * s2 + s1 * s3),
            0.0f,
        },
        {
            scale.z * (c2 * s1),
            scale.z * (-s2),
            scale.z * (c1 * c2),
            0.0f,
        },
        {translation.x, translation.y, translation.z, 1.0f} 
    };
}

glm::mat3 Transform::normalMatrix() const {
    //const float s1 = glm::sin(rotation.x);
    //const float c1 = glm::cos(rotation.x);
    //const float s2 = glm::sin(rotation.y);
    //const float c2 = glm::cos(rotation.y);
    //const float s3 = glm::sin(rotation.z);
    //const float c3 = glm::cos(rotation.z);
    //glm::vec3 invScale = 1.0f / scale;

    //return glm::mat3{
    //    {
    //        invScale.x * (c2 * c3),
    //        invScale.x * (c1 * s3 + c3 * s1 * s2),
    //        invScale.x * (s1 * s3 - c1 * c3 * s2)
    //    },
    //    {
    //        invScale.y * (-c2 * s3),
    //        invScale.y * (c1 * c3 - s1 * s2 * s3),
    //        invScale.y * (c3 * s1 + c1 * s2 * s3)
    //    },
    //    {
    //        invScale.z * (s2),
    //        invScale.z * (-c2 * s1),
    //        invScale.z * (c1 * c2)
    //    }
    //};


    const float c3 = glm::cos(rotation.z);
    const float s3 = glm::sin(rotation.z);
    const float c2 = glm::cos(rotation.x);
    const float s2 = glm::sin(rotation.x);
    const float c1 = glm::cos(rotation.y);
    const float s1 = glm::sin(rotation.y);
    glm::vec3 invScale = 1.0f / scale;

    return glm::mat3{
        {
            invScale.x * (c1 * c3 + s1 * s2 * s3),
            invScale.x * (c2 * s3),
            invScale.x * (c1 * s2 * s3 - c3 * s1),
        },
        {
            invScale.y * (c3 * s1 * s2 - c1 * s3),
            invScale.y * (c2 * c3),
            invScale.y * (c1 * c3 * s2 + s1 * s3),
        },
        {
            invScale.z * (c2 * s1),
            invScale.z * (-s2),
            invScale.z * (c1 * c2),
        }
    };
}