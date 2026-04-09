#pragma once

#include "model.hpp"

#include "glm/gtc/matrix_transform.hpp"

#include <memory>

struct TransformComponent {
	glm::vec3 translation;
	glm::vec3 scale{ 1.f, 1.f, 1.f };
	glm::vec3 rotation;

    // Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
    // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
    // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
	glm::mat4 mat4() const;

	glm::mat3 normalMatrix() const;
};

class GameObject {
public:
	using id_t = unsigned int;

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

	std::shared_ptr<Model> model{};
	glm::vec3 color{};
	TransformComponent transform{};

private:
    GameObject(id_t objId) : id{ objId } {}

	id_t id;
};