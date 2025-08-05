#ifndef TEXTURES_HPP
#define TEXTURES_HPP
#pragma once
#include <vector>
#include <glad/glad.h>

struct Texture {
    unsigned int id;
    GLint64 handle;
};

class Textures {
public:
    std::vector<Texture*> textures;

    Texture* load_texture(const char* file_path, bool generate_minimap = true);
    ~Textures();
};

#endif //TEXTURES_HPP
