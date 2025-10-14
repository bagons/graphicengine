#ifndef SHADERS_HPP
#define SHADERS_HPP
#pragma once
#include <span>
#include <vector>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <string>
#include <variant>
#include <memory>
#include <map>

typedef std::variant<float, glm::vec3> uniform_variant;
typedef std::map<int, uniform_variant> uniform_map;


class Shader {
public:
    unsigned int id = -1;
    GLenum shader_type = 0;

    Shader(const char* file_path, GLenum shader_type);
    Shader(const std::string &shader_code, GLenum _shader_type);
    ~Shader();
};


class ShaderProgram {
public:
    unsigned int id = -1;

    explicit ShaderProgram(unsigned int _id);
    ShaderProgram(const ShaderProgram& sp);
    ShaderProgram(const Shader &vertex_shader, const Shader &fragment_shader);
    ~ShaderProgram();
    void use() const;

    void set_uniform(const char* uniform_name, glm::mat4 matrix) const;
    void set_uniform(const char* uniform_name, GLint64 texture_handle) const;
    void set_uniform(const char* uniform_name, glm::vec3 val) const;
    void set_uniform(const char* uniform_name, float val) const;

    ShaderProgram& operator=(const ShaderProgram& sp);

    unsigned int get_uniform_location(const char *uniform_name) const;
};


// HOLDS MATERIAL UNIFORM VALUES
class Material {
public:
    ShaderProgram shader_program;
    uniform_map shader_values = {};
    explicit Material(const ShaderProgram &_shader_program);

    void set_uniform_values() const;

    void save_uniform_value(const char* uniform_name, uniform_variant val);

    unsigned int get_uniform_location(const char *uniform_name) const;
};

struct MaterialSorter {
    bool operator()(const std::shared_ptr<Material>& m1, const std::shared_ptr<Material>& m2) const {
        return m1->shader_program.id < m2->shader_program.id;
    }
};


class Shaders {
    std::map<unsigned int, unsigned int> shader_programs_id_used = {};
public:
    void add_shader_id_use(unsigned int sp_id);
    void remove_shader_id_use(unsigned int sp_id);
};


class ShaderGen {
public:
    static ShaderProgram phong_shader_program_gen(bool has_uvs);
    static Shader base_vertex_shader_gen(bool support_uv = true, bool support_normal = true);
    static Shader base_phong_shader_gen(bool support_uv = true);
    static std::string parse_shader_template(const char* file_path, std::string flags_to_include);
};

#endif //SHADERS_HPP
