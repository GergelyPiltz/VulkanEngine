#pragma once
#include "model.hpp"
#include "texture.hpp"

// libs
#include "glm/gtc/matrix_transform.hpp"

//std
#include <memory>
#include <unordered_map>

struct Transform {
	Transform() = default;

	glm::vec3 position = { 0.0f, 0.0f, 0.0f };
	glm::vec3 scale = { 1.0f, 1.0f, 1.0f };
	glm::vec3 rotation = { 0.0f, 0.0f, 0.0f };

    // Matrix corresponds to Translate * Ry * Rx * Rz * Scale
	// Rotations correspond to YAW-PITCH-ROLL in that order which is Tait-bryan angles of Y(1), X(2), Z(3)
    // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
	// Vulkans internal coordinate system makes:
	// YAW = Y
	// PITCH = X
	// ROLL = Z
	//	        Rotations are clockwise facing the positive direction
	//          
	//         / +Z
	//        /
	//       /
	//      0--------- +X
	//      |
	//      |
	//      |
	//      | +Y


	glm::mat4 modelMatrix() const;

	glm::mat3 normalMatrix() const;
};

struct PointLight {
	glm::vec4 color{ 1.0f };
};

struct RigidBody {
	glm::vec3 velocity{ 0.0f };
	glm::vec3 centerOfMass{ 0.0f };
	float mass = 0.0f;
};

struct SphereCollider {
	glm::vec3 center{ 0.0f };
	float radius = 0.0f;
};

struct BoxCollider {
	glm::vec3 minExtent{ 0.0f };
	glm::vec3 maxExtent{ 0.0f };
};

struct MeshCollider {
	std::vector<glm::vec3> vertices;
};

class GameObject {
public:
	using id_t = unsigned int;
	using Map = std::unordered_map<id_t, GameObject>;

	static GameObject createGameObject() {
		static id_t currentId = 0;
		return GameObject{ currentId++ };
	}

	// Not copyable but movable
	GameObject(const GameObject&) = delete;
	GameObject& operator=(const GameObject&) = delete;
	GameObject(GameObject&&) = default;
	GameObject& operator=(GameObject&&) = default;

	id_t getId() const { return id; }

	uint32_t textureIndex = 0;


	std::unique_ptr<Transform> transform;
	std::shared_ptr<Model> model;
	//std::shared_ptr<Model> wireFrame;
	std::unique_ptr<PointLight> pointLight;
	std::unique_ptr<RigidBody> rigidBody;
	std::unique_ptr<SphereCollider> sphereCollider;
	std::unique_ptr<BoxCollider> boxCollider;
	std::unique_ptr<MeshCollider> meshCollider;

private:
    GameObject(id_t objId) : id{ objId } {}

	id_t id;
};