#ifndef TEXTURES_HPP
#define TEXTURES_HPP
#pragma once
#include <unordered_map>
#include <vector>
#include <glad/glad.h>

unsigned int setup_texture_from_file(const char* file_path, bool generate_minimaps);

struct TextureRef {
    unsigned int id;
    GLint64 handle;
    ~TextureRef();
    TextureRef& operator=(const TextureRef& ref);
};

class Textures {
public:
    std::unordered_map<GLint64, unsigned int> texture_reference_count;
    std::unordered_map<GLint64, unsigned int> texture_handle_to_id;

    TextureRef load(const char* file_path, bool generate_minimaps = true);
    void call_of_texture_reference(GLint64 handle);
    void call_of_texture_reference(const TextureRef& ref);
    void add_use_of_texture_reference(const TextureRef& ref);
};

#endif //TEXTURES_HPP
