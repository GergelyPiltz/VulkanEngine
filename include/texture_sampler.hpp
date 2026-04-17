#pragma once

#include "device.hpp"

class TextureSampler {
public:
	TextureSampler(Device& device);
	~TextureSampler();

	VkSampler textureSampler() const { return m_textureSampler; }

private:
	Device& m_device;
	VkSampler m_textureSampler;
};