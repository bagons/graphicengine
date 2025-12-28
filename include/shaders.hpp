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
#include <array>
#include "textures.hpp"

typedef std::variant<float, glm::vec3, GLint64, bool>  uniform_variant;
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
    ~Material();

    void set_uniform_values() const;

    void save_uniform_value(const char* uniform_name, const uniform_variant &val);

    void save_uniform_value(const char* uniform_name, const TextureRef &texture);

    unsigned int get_uniform_location(const char *uniform_name) const;
};

struct MaterialSorter {
    bool operator()(const std::shared_ptr<Material>& m1, const std::shared_ptr<Material>& m2) const {
        return m1->shader_program.id < m2->shader_program.id;
    }
};


class Shaders {
    std::map<unsigned int, unsigned int> shader_programs_id_used = {};

    // BASE SHADER PROGRAMS
    // a set of shader programs useful for loading MTL materials
    // all here, so that they can be universally used across all loaded models
    // but! we only instance them if they are needed, otherwise we keep the memory empty
    // legenda:
    // 0 -> v
    // 1 -> vn
    // 2 -> vu
    // 3 -> vnu
    // PS: these sp can be useful if you don't want to use custom shaders
    std::array<std::shared_ptr<Material>, 4> base_materials{};
public:
    void setup_base_materials();
    void add_shader_id_use(unsigned int sp_id);
    void remove_shader_id_use(unsigned int sp_id);

    //base shader ps
    std::shared_ptr<Material> get_base_material(bool with_uvs, bool with_normals);
    void clear_base_material(size_t index);
};


class ShaderGen {
public:
    static ShaderProgram phong_shader_program_gen(bool has_uvs);
    static ShaderProgram no_normal_program_gen(bool has_uvs);

    static Shader base_vertex_shader_gen(bool support_uv = true, bool support_normal = true);
    static Shader base_phong_shader_gen(bool support_uv = true);
    static Shader base_no_normal_shader_gen(bool support_uv = true);
    static std::string parse_shader_template(const char* file_path, std::string flags_to_include);
};

#endif //SHADERS_HPP
