#pragma once

#include "game_object.hpp"
#include "frame_info.hpp"

// std
#include <memory>
#include <vector>

class PhysicsSystem {
public:
	PhysicsSystem() {};
	~PhysicsSystem() {};

	PhysicsSystem(const PhysicsSystem&) = delete;
	PhysicsSystem& operator=(const PhysicsSystem&) = delete;
	PhysicsSystem(PhysicsSystem&&) = delete;
	PhysicsSystem& operator=(PhysicsSystem&&) = delete;

	void detect(FrameInfo frameInfo);
	void update(FrameInfo frameInfo);

private:

};