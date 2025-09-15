#ifndef SHADERS_HPP
#define SHADERS_HPP
#pragma once
#include <span>
#include <vector>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <string>
#include <variant>
#include <map>

typedef std::map<int, std::variant<float, glm::vec3>> uniform_map;

class ShaderProgram {
public:
    unsigned int id = -1;
    std::vector<unsigned int> quick_uniform_ids;
    ShaderProgram(unsigned int vertex_shader, unsigned int fragment_shader);
    ShaderProgram(unsigned int vertex_shader, unsigned int fragment_shader, const std::vector<const char *>& quick_uniforms);
    void use();

    void set_uniform(const char* uniform_name, glm::mat4 matrix) const;
    void set_uniform(const char* uniform_name, GLint64 texture_handle) const;
    void set_uniform(const char* uniform_name, glm::vec3 val) const;
    void set_uniform(const char* uniform_name, float val) const;
};


// HOLDS MATERIAL UNIFORM VALUES
class Material {
public:
    ShaderProgram *shader_program;
    uniform_map shader_values;
    Material(ShaderProgram* _shader_program, uniform_map& _shader_values);

    void set_uniform_values() const;
};

struct MaterialSorter {
    bool operator()(const Material* m1, const Material* m2) const {
        return m1->shader_program->id < m2->shader_program->id;
    }
};

class Shaders {
public:
    std::vector<ShaderProgram *> programs;
};

class ShaderGen {
public:
    static ShaderProgram phong_shader_program_gen(bool has_uvs);
    static unsigned int base_vertex_shader_gen(bool support_uv = true, bool support_normal = true);
    static unsigned int base_phong_shader_gen(bool support_uv = true);
    static std::string parse_shader_template(const char* file_path, std::string flags_to_include);
};

#endif //SHADERS_HPP
