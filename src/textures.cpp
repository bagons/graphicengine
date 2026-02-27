#include "textures.hpp"
#include <iostream>
#include <graphicengine.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "../utils/stb_image.h"

constexpr int channel_to_format[4] ={
    GL_RED,
    GL_RGB,
    GL_RGB,
    GL_RGBA
};

constexpr int channel_internal_formats[4] ={
    GL_R8,
    GL_RGB,
    GL_RGB,
    GL_RGBA
};

unsigned int setup_texture_from_file(const char* file_path, const bool sRGB, const bool generate_minimaps, const bool clamp) {
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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clamp ? GL_CLAMP_TO_BORDER : GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clamp ? GL_CLAMP_TO_BORDER : GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (channel_count == 2) {
        std::cerr << "ENGINE ERROR: Texture has 2 channels ? " << file_path << std::endl;
        return 0;
    }

    if (sRGB and channel_count < 3) {
        std::cerr << "ENGINE ERROR: sRGB not supported on textures with 1 channel" << file_path << std::endl;
        return 0;
    }

    const auto format = channel_to_format[channel_count - 1];
    const auto internal_format = !sRGB ? channel_internal_formats[channel_count - 1] : (channel_count == 4 ? GL_SRGB_ALPHA : GL_SRGB);

    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

    if (generate_minimaps) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    stbi_image_free(data);
    return texture;
}


Texture::Texture(const char* file_path, bool sRGB, bool generate_minimap, bool clamp) {
    id = setup_texture_from_file(file_path, sRGB, generate_minimap, clamp);
    Engine::debug_message("Loaded texture: " + std::string(file_path) + " id: " + std::to_string(id));
    generate_bindless_handle();
}

Texture::Texture(Color color, const bool alpha) {
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    const int format = alpha ? GL_RGBA : GL_RGB;

    const auto data = new unsigned char[alpha ? 4 : 3];

    color.r = std::max(0.0f, std::min(1.0f, color.r));
    data[0] = static_cast<unsigned char>(floor(color.r >= 1.0 ? 255 : color.r * 256.0));
    color.g = std::max(0.0f, std::min(1.0f, color.g));
    data[1] = static_cast<unsigned char>(floor(color.g >= 1.0 ? 255 : color.g * 256.0));
    color.b = std::max(0.0f, std::min(1.0f, color.b));
    data[2] = static_cast<unsigned char>(floor(color.b >= 1.0 ? 255 : color.b * 256.0));

    if (alpha) {
        color.a = std::max(0.0f, std::min(1.0f, color.a));
        data[3] = static_cast<unsigned char>(floor(color.a >= 1.0 ? 255 : color.a * 256.0));
    }

    glTexImage2D(GL_TEXTURE_2D, 0, format, 1, 1, 0, format, GL_UNSIGNED_BYTE, data);
    generate_bindless_handle();
}

void Texture::generate_bindless_handle() {
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

