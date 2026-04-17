#pragma once

#include "device.hpp"

class Texture {
public:
    Texture(Device& device);
    ~Texture();

    VkImageView textureImageView() const { return m_textureImageView; }

private:
    Device& device;
    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    VkImageView m_textureImageView;

    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    VkImageView createImageView(VkImage image, VkFormat format);
};