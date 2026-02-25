#include "shaders.hpp"
#include <iostream>
#include <fstream>
#include <utility>
#include "graphicengine.hpp"
#include "gtc/type_ptr.hpp"


Shader::Shader(const char *file_path, const ShaderType shader_type, const std::string &define_header) {
    id = glCreateShader(shader_type);

    std::string shader_string;

    std::ifstream file(file_path);

    if (!file.good()) {
        glDeleteShader(id);
        Engine::debug_error("Failed to load shader file: " + std::string(file_path));
        return;
    }

    std::string line;
    bool prepended_header = false;
    while (std::getline(file, line)) {
        shader_string += line + '\n';
        if (!prepended_header and line[0] == '#' and line[1] == 'v') {
            prepended_header = true;
            shader_string += define_header;

            // add obligatory header (if not needed preprocessor will strip it away anyway)
            if (shader_type == GL_FRAGMENT_SHADER) {
                shader_string += "#define NR_POINT_LIGHTS " + std::to_string(ge.lights.MAX_NR_POINT_LIGHTS) + "\n";
                shader_string += "#define NR_DIRECTIONAL_LIGHTS " + std::to_string(ge.lights.MAX_NR_DIRECTIONAL_LIGHTS) + "\n";
            }
        }
    }
    file.close();

    file_path = shader_string.c_str();
    glShaderSource(id, 1, &file_path, nullptr);
    glCompileShader(id);

    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(id, 512, nullptr, infoLog);
        Engine::debug_error("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" + std::string(infoLog));
    } else {
        Engine::debug_message("Shader successfully compiled!");
    }
}


Shader::Shader(const std::string &shader_code, const ShaderType shader_type) {
    id = glCreateShader(shader_type);
    const char *content = shader_code.c_str();
    glShaderSource(id, 1, &content, nullptr);
    glCompileShader(id);


    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(id, 512, nullptr, infoLog);
        Engine::debug_error("ERROR::SHADER::COMPILATION_FAILED\n" + std::string(infoLog));
    } else {
        Engine::debug_message("Shader successfully compiled from std::string");
    }
}

unsigned int Shader::get_id() const {
    return id;
}


Shader::~Shader() {
    glDeleteShader(id);
}


ShaderProgram::ShaderProgram(unsigned int _id) {
    id = _id;
    ge.shaders.add_shader_id_use(id);
}

ShaderProgram::ShaderProgram(const ShaderProgram &sp) {
    id = sp.id;
    ge.shaders.add_shader_id_use(id);
}

ShaderProgram::ShaderProgram(ShaderProgram &&other) noexcept {
    id = other.id;
    other.id = -1;
}

ShaderProgram& ShaderProgram::operator=(const ShaderProgram &other) {
    ge.shaders.remove_shader_id_use(id);
    id = other.id;
    ge.shaders.add_shader_id_use(id);
    return *this;
}


ShaderProgram& ShaderProgram::operator=(ShaderProgram &&other) noexcept {
    if (this == &other)
        return *this;

    ge.shaders.remove_shader_id_use(id);
    id = other.id;
    other.id = -1;

    return *this;
}

ShaderProgram::~ShaderProgram() { // DESTRUCTOR WARNING !!
    ge.shaders.remove_shader_id_use(id);
}


ShaderProgram::ShaderProgram(const Shader &vertex_shader, const Shader &fragment_shader) {
    id = glCreateProgram();
    glAttachShader(id, vertex_shader.get_id());
    glAttachShader(id, fragment_shader.get_id());
    glLinkProgram(id);

    int success;
    char infoLog[512];
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(id, 512, nullptr, infoLog);
        Engine::debug_error("Program link error:\n" + std::string(infoLog));
    }

    glUniformBlockBinding(id, glGetUniformBlockIndex(id, "MATRICES"), 0);

    const auto light_block_idx = glGetUniformBlockIndex(id, "LIGHTS");
    if (light_block_idx != GL_INVALID_INDEX) {
        glUniformBlockBinding(id, light_block_idx, 1);
    }

    ge.shaders.add_shader_id_use(id);
}

unsigned int ShaderProgram::get_id() const {
    return id;
}

void ShaderProgram::use() const {
    glUseProgram(id);
}

void ShaderProgram::set_uniform(const char* uniform_name, glm::mat4 matrix) const {
    glUniformMatrix4fv(glGetUniformLocation(id, uniform_name), 1, GL_FALSE, &matrix[0][0]);
}

void ShaderProgram::set_uniform(const char* uniform_name, float val) const {
    glUniform1f(glGetUniformLocation(id, uniform_name), val);
}

void ShaderProgram::set_uniform(const char* uniform_name, glm::vec3 val) const {
    glUniform3fv(glGetUniformLocation(id, uniform_name), 1, &val[0]);
}

unsigned int ShaderProgram::get_uniform_location(const char *uniform_name) const {
    return glGetUniformLocation(id, uniform_name);
}

Material::Material(const ShaderProgram &_shader_program) : shader_program(_shader_program) {
    id = ge.shaders.get_material_identificator();
}

void Material::set_uniform_values() const {
    uniform_map::const_iterator it;

    // (only used when bindless textures are NOT supported)
    int bind_texture_slot = 0;

    for (it = shader_values.begin(); it != shader_values.end(); it++)
    {
        if (std::holds_alternative<float>(it->second)) {
            glUniform1f(it->first, std::get<float>(it->second));
        }
        else if (std::holds_alternative<Vector3>(it->second)) {
            const auto vec = std::get<Vector3>(it->second);
            glUniform3f(it->first, vec.x, vec.y, vec.z);
        }
        else if (std::holds_alternative<std::shared_ptr<Texture>>(it->second)) {
            if (ge.are_bindless_textures_supported()) {
                glProgramUniformHandleui64ARB(shader_program.get_id(), it->first, std::get<std::shared_ptr<Texture>>(it->second)->handle);
            } else {
                glActiveTexture(GL_TEXTURE0 + bind_texture_slot);
                glBindTexture(GL_TEXTURE_2D, std::get<std::shared_ptr<Texture>>(it->second)->id);
                glUniform1i(it->first, bind_texture_slot);
                bind_texture_slot += 1;
            }
        } else if (std::holds_alternative<bool>(it->second)) {
            glUniform1i(it->first, std::get<bool>(it->second));
        }
    }
}


unsigned int Material::get_uniform_location(const char *uniform_name) const {
    shader_program.use();
    return glGetUniformLocation(shader_program.get_id(), uniform_name);
}

void Material::save_uniform_value(const char *uniform_name, const uniform_variant &val) {
    shader_values[glGetUniformLocation(shader_program.get_id(), uniform_name)] = val;
}


void Shaders::add_shader_id_use(const unsigned int sp_id) {
    shader_programs_id_used[sp_id] += 1;
}

void Shaders::remove_shader_id_use(const unsigned int sp_id) {
    if (sp_id == -1) {
        return;
    }
    if (!shader_programs_id_used.contains(sp_id)) {
        Engine::debug_error("sp_id: " + std::to_string(sp_id) + " NOT FOUND !! ");
        return;
    }
    shader_programs_id_used[sp_id] -= 1;

    if (shader_programs_id_used[sp_id] == 0) {
        glDeleteProgram(sp_id);
        shader_programs_id_used.erase(sp_id);
        Engine::debug_message("deleting shader program " + std::to_string(sp_id));
    }
}

unsigned int Shaders::get_shader_use_by_id(unsigned int sp_id) {
    auto it = shader_programs_id_used.find(sp_id);
    if (it != shader_programs_id_used.end())
        return it->second;
    return -1;
}


void Shaders::debug_show_shader_program_use() {
    std::cout << "ENGINE MESSAGE: SHADER PROGRAM USE DEBUG" << std::endl;
    std::cout << "! - ID X USE COUNT -  !" << std::endl;
    for (auto it = shader_programs_id_used.begin(); it != shader_programs_id_used.end(); it++) {
        std::cout << it->first << " X " << it->second << std::endl;
    }
}


void Shaders::setup_base_materials() {
    base_materials[0] = std::make_shared<Material>(phong_shader_program_gen(true));
    base_materials[0]->save_uniform_value("material.ambient", Vector3(0.2f, 0.2f, 0.2f));
    base_materials[0]->save_uniform_value("material.diffuse", Vector3(1.0f, 1.0f, 1.0f));
    base_materials[0]->save_uniform_value("material.specular", Vector3(1.0f, 1.0f, 1.0f));
    base_materials[0]->save_uniform_value("material.shininess", 32.0f);
    base_materials[0]->save_uniform_value("material.has_albedo", false);

    base_materials[1] = std::make_shared<Material>(phong_shader_program_gen(false));
    base_materials[1]->save_uniform_value("material.ambient", Vector3(0.2f, 0.2f, 0.2f));
    base_materials[1]->save_uniform_value("material.diffuse", Vector3(1.0f, 1.0f, 1.0f));
    base_materials[1]->save_uniform_value("material.specular", Vector3(1.0f, 1.0f, 1.0f));
    base_materials[1]->save_uniform_value("material.shininess", 32.0f);
    base_materials[1]->save_uniform_value("material.has_albedo", false);

    base_materials[2] = std::make_shared<Material>(no_normal_program_gen(true));
    base_materials[2]->save_uniform_value("material.diffuse", Vector3(1.0f, 1.0f, 1.0f));
    base_materials[2]->save_uniform_value("material.has_albedo", false);

    base_materials[3] = std::make_shared<Material>(no_normal_program_gen(false));
    base_materials[3]->save_uniform_value("material.diffuse", Vector3(1.0f, 1.0f, 1.0f));
    base_materials[3]->save_uniform_value("material.has_albedo", false);
}

std::shared_ptr<Material> Shaders::get_base_material(const bool with_uvs, const bool with_normals) {
    if (with_uvs and with_normals)
        return base_materials[0];
    if (with_normals)
        return base_materials[1];
    if (with_uvs)
        return base_materials[2];
    return base_materials[3];
}

void Shaders::clear_base_material(const size_t index) {
    base_materials[index] = nullptr;
}

uint64_t Shaders::get_material_identificator() {
    next_material_id += 1;
    return next_material_id - 1;
}



// SHADER GEN
Shader Shaders::base_vertex_shader_gen(const bool support_uv, const bool support_normal) {
    std::string define_header;
    if (support_uv)
        define_header += "#define HAS_UV\n";
    if (support_normal)
        define_header += "#define HAS_NORMALS\n";

    return Shader{"engine/res/shaders/vertex_shader_template.glsl", Shader::VERTEX_SHADER, define_header};
}


Shader Shaders::base_phong_shader_gen(const bool support_uv) const {
    std::string define_header = support_uv ? "#define HAS_UV\n" : "";
    if (bindless_textures_supported)
        define_header += "#define USE_BINDLESS\n";
    return Shader{"engine/res/shaders/obj_phong.glsl", Shader::FRAGMENT_SHADER, define_header};
}


Shader Shaders::base_no_normal_shader_gen(bool support_uv) const {
    std::string define_header = support_uv ? "#define HAS_UV\n" : "";
    if (bindless_textures_supported)
        define_header += "#define USE_BINDLESS\n";
    return Shader{"engine/res/shaders/obj_no_normal.glsl", Shader::FRAGMENT_SHADER, define_header};
}


ShaderProgram Shaders::phong_shader_program_gen(bool has_uvs) const {
    ShaderProgram sp {base_vertex_shader_gen(has_uvs, true), base_phong_shader_gen(has_uvs)};
    return sp;
}


ShaderProgram Shaders::no_normal_program_gen(bool has_uvs) const {
    ShaderProgram sp {base_vertex_shader_gen(has_uvs, false), base_no_normal_shader_gen(has_uvs)};
    return sp;
}
