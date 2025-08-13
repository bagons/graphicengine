#ifndef SHADERS_HPP
#define SHADERS_HPP
#pragma once
#include <span>
#include <vector>
#include <glm/glm.hpp>
#include <glad/glad.h>

class ShaderProgram {
public:
    unsigned int id = -1;
    std::vector<unsigned int> quick_uniform_ids;
    ShaderProgram(unsigned int vertex_shader, unsigned int fragment_shader, std::span<const char *> quick_uniforms);
    void use();

    void set_uniform(const char* uniform_name, glm::mat4 matrix) const;
    void set_uniform(const char* uniform_name, GLint64 texture_handle) const;
    void set_uniform(const char* uniform_name, glm::vec3) const;
    void set_uniform(const char* uniform_name, float val) const;
};

class Shaders {
public:
    std::vector<ShaderProgram *> programs;
};

#endif //SHADERS_HPP
