#ifndef TEXTURES_HPP
#define TEXTURES_HPP
#pragma once
#include <unordered_map>
#include <glad/glad.h>

unsigned int setup_texture_from_file(const char* file_path, bool generate_minimaps);

class Texture {
public:
    unsigned int id;
    GLint64 handle;
    explicit Texture(const char* file_path, bool generate_minimap = true);
    ~Texture();
};

#endif //TEXTURES_HPP
