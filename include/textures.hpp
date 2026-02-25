#ifndef TEXTURES_HPP
#define TEXTURES_HPP
#pragma once
#include <glad/glad.h>

/// A texture GPU resource a wrapper around OpenGL texture ID system, supports normal and also bindless textures
class Texture {
public:
    /// standard OpenGL texture id
    unsigned int id;
    /// handle for bindless textures
    GLint64 handle;
    /// Loads texture from file and uploads it to the GPU.
    /// @note Supports RGB and RGBA formats.
    /// @warning If you construct the same texture twice, there is no checking as of this point, so it'll be allocated twice on the GPU.
    /// @warning If you want to use this texture twice just give the std::shared_ptr to two materials.
    explicit Texture(const char* file_path, bool sRGB = false, bool generate_minimap = true);
    /// Deconstructs and removes the texture from GPU.
    ~Texture();
};

#endif //TEXTURES_HPP
