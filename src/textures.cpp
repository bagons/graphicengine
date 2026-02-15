#include "textures.hpp"
#include <iostream>
#include <graphicengine.hpp>

#pragma once
#define STB_IMAGE_IMPLEMENTATION
#include "../utils/stb_image.h"

unsigned int setup_texture_from_file(const char* file_path, bool generate_minimaps) {
    unsigned int texture;
    glGenTextures(1, &texture);
    int width, height, channel_count;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(file_path, &width, &height, &channel_count, 0);

    if (data == nullptr) {
        Engine::debug_error("Failed to load texture from file: " + std::string(file_path));
    }

    glBindTexture(GL_TEXTURE_2D, texture);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    if (channel_count < 3) {
        std::cerr << "ENGINE ERROR: Less then 3 channels (" << channel_count << ")" << " in texture, WTF? -> continuing as RGB format, may lead to crashes, beware"<< std::endl;
    }

    const auto format = channel_count == 4 ? GL_RGBA : GL_RGB;

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

    if (generate_minimaps) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    stbi_image_free(data);
    return texture;
}


Texture::Texture(const char* file_path, bool generate_minimap) {
    id = setup_texture_from_file(file_path, generate_minimap);

    if (ge.are_bindless_textures_supported()) {
        handle = glGetTextureHandleARB(id);
        glMakeTextureHandleResidentARB(handle);

        if (!glIsTextureHandleResidentARB(handle)) {
            std::cerr << "ENGINE ERROR: Texture handle is NOT resident!" << std::endl;
        }
    }
}

Texture::~Texture() {
    if (ge.are_bindless_textures_supported()) {
        glMakeTextureHandleNonResidentARB(handle);
    }
    glDeleteTextures(1, &id);
    std::cout << "ENGINE MESSAGE: Deleting texture " << id << "/" << handle << std::endl;
}

