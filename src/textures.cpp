#include "textures.hpp"
#include <iostream>
#include <gameengine.hpp>

#pragma once
#define STB_IMAGE_IMPLEMENTATION
#include "../utils/stb_image.h"

unsigned int setup_texture_from_file(const char* file_path, bool generate_minimaps) {
    unsigned int texture;
    glGenTextures(1, &texture);
    int width, height, channel_count;
    unsigned char* data = stbi_load(file_path, &width, &height, &channel_count, 0);

    if (data == nullptr) {
        std::cout << "Failed to load texture from file: " << file_path << std::endl;
    }

    glBindTexture(GL_TEXTURE_2D, texture);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load texture to gpu
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    if (generate_minimaps) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    stbi_image_free(data);
    return texture;
}

TextureRef Textures::load(const char* file_path, bool generate_minimap) {
    TextureRef texture;
    texture.id = setup_texture_from_file(file_path, generate_minimap);

    texture.handle = glGetTextureHandleARB(texture.id);
    glMakeTextureHandleResidentARB(texture.handle);

    if (!glIsTextureHandleResidentARB(texture.handle)) {
        std::cout << "Texture handle is NOT resident!" << std::endl;
    } else {
        std::cout << "Texture handle is resident!" << std::endl;
    }

    add_use_of_texture_reference(texture);

    return texture;
}

void Textures::add_use_of_texture_reference(const TextureRef& ref) {
    texture_handle_to_id[ref.handle] = ref.id;
    texture_reference_count[ref.handle] += 1;
}

void Textures::call_of_texture_reference(const TextureRef& ref) {
    texture_reference_count[ref.id] -= 1;

    if (texture_reference_count[ref.id] == 0) {
        glMakeTextureHandleNonResidentARB(ref.handle);
        glDeleteTextures(1, &ref.id);
        std::cout << "deleting texture " << ref.id << " " << ref.handle << std::endl;
    }
}

void Textures::call_of_texture_reference(const GLint64 handle) {
    texture_reference_count[handle] -= 1;

    if (texture_reference_count[handle] == 0) {
        glMakeTextureHandleNonResidentARB(handle);
        glDeleteTextures(1, &texture_handle_to_id[handle]);
    }
}


TextureRef::~TextureRef() { // DESTRUCTOR WARNING
    ge.textures.call_of_texture_reference(*this);
}

TextureRef& TextureRef::operator=(const TextureRef& ref) {
    id = ref.id;
    handle = ref.handle;
    ge.textures.add_use_of_texture_reference(*this);
    return *this;
}