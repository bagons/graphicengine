#include "shaders.hpp"
#include <iostream>
#include <fstream>

#include "gameengine.hpp"
#include "utils.h"


Shader::Shader(const char *file_path, const GLenum _shader_type) {
    shader_type = _shader_type;
    id = glCreateShader(_shader_type);

    std::cout << "loading direct shader file: " << file_path << std::endl;
    std::string shader_string;

    std::ifstream file(file_path);
    std::cout << "file_read_state: " << file.good() << std::endl;

    if (!file.good()) {
        glDeleteShader(id);
        std::cout << "failed to load file: " << file_path << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        shader_string += line + '\n';
    }
    file.close();

    file_path = shader_string.c_str();
    glShaderSource(id, 1, &file_path, nullptr);
    glCompileShader(id);
    std::cout << "file: " << std::endl << file_path << std::endl;


    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(id, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    } else {
        std::cout << "Shader successfully compiled" << std::endl;
    }
}


Shader::Shader(const std::string &shader_code, const GLenum _shader_type) {
    shader_type = _shader_type;
    id = glCreateShader(_shader_type);
    const char *content = shader_code.c_str();
    glShaderSource(id, 1, &content, nullptr);
    glCompileShader(id);


    //std::cout << "SHADER CODE: " << content << std::endl;

    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(id, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    } else {
        std::cout << "Shader successfully compiled from std::string" << std::endl;
    }
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


ShaderProgram::ShaderProgram(const Shader &vertex_shader, const Shader &fragment_shader) {
    id = glCreateProgram();
    glAttachShader(id, vertex_shader.id);
    std::cout << "attaching vertex shader" << vertex_shader.id << std::endl;
    glAttachShader(id, fragment_shader.id);
    std::cout << "attaching fragment shader" << fragment_shader.id << std::endl;
    glLinkProgram(id);

    int success;
    char infoLog[512];
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(id, 512, NULL, infoLog);
        std::cout << "Program link error:\n" << infoLog << std::endl;
    }

    ge.shaders.add_shader_id_use(id);
}


ShaderProgram::~ShaderProgram() {
    ge.shaders.remove_shader_id_use(id);
}



void ShaderProgram::use() const {
    glUseProgram(id);
}

void ShaderProgram::set_uniform(const char* uniform_name, glm::mat4 matrix) const {
    glUniformMatrix4fv(glGetUniformLocation(id, uniform_name), 1, GL_FALSE, &matrix[0][0]);
}
void ShaderProgram::set_uniform(const char* uniform_name, GLint64 texture_handle) const {
    std::cout << texture_handle << std::endl;
    const int uniform_loc = glGetUniformLocation(id, uniform_name);
    std::cout << "uniform loc: " << uniform_loc << std::endl;
    glUniformHandleui64ARB(uniform_loc, texture_handle);
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

ShaderProgram& ShaderProgram::operator=(const ShaderProgram &other) {
    id = other.id;
    ge.shaders.add_shader_id_use(id);
    return *this;
}


Material::Material(const ShaderProgram &_shader_program) : shader_program(_shader_program) {

}

void Material::set_uniform_values() const {
    uniform_map::const_iterator it;

    for (it = shader_values.begin(); it != shader_values.end(); it++)
    {
        if (std::holds_alternative<float>(it->second))
            glUniform1f(it->first, std::get<float>(it->second));
        else if (std::holds_alternative<glm::vec3>(it->second))
            glUniform3fv(it->first, 1, &std::get<glm::vec3>(it->second)[0]);
    }
}


unsigned int Material::get_uniform_location(const char *uniform_name) const {
    return glGetUniformLocation(shader_program.id, uniform_name);
}

void Material::save_uniform_value(const char *uniform_name, const uniform_variant val) {
    shader_values[glGetUniformLocation(shader_program.id, uniform_name)] = val;
}



void Shaders::add_shader_id_use(const unsigned int sp_id) {
    shader_programs_id_used[sp_id] += 1;
    std::cout << "adding use - shader_program_id: " << sp_id << std::endl;
}

void Shaders::remove_shader_id_use(const unsigned int sp_id) {
    shader_programs_id_used[sp_id] -= 1;

    if (shader_programs_id_used[sp_id] == 0) {
        //glDeleteProgram(sp_id);
        std::cout << "deleting " << sp_id << std::endl;
    }
}



// SHADER GEN

std::string ShaderGen::parse_shader_template(const char * file_path, const std::string flags_to_include) {
    std::string shader_string;

    std::ifstream file(file_path);
    std::cout << "file_read_state: " << file.good() << std::endl;
    std::string line;
    while (std::getline(file, line)) {
        // if template line

        if (line[0] == '>' || line[0] == '!') {
            size_t i = 0;
            bool passed = true;
            while (line[i] == '>' || line[i] == '!') {
                // # means flag should be present, ! means flag should NOT be present
                // do we want a match or no
                bool match = line[i] == '>';
                //std::cout << line[i] << std::endl;

                bool found_a_letter = false;
                // loop through all flags
                for (auto letter : flags_to_include) {
                    std::cout << letter << std::endl;
                    // if NOT maching and we find it, hard stop.
                    if (!match && line[i + 1] == letter) {
                        passed = false;
                        break;
                        // if matchng and we found match great!
                    } else if (match && line[i + 1] == letter) {
                        found_a_letter = true;
                        break;
                    }
                }

                // hard stop
                if (!passed)
                    break;

                // if maching and we didn't find anything, hard stop.
                if (match && !found_a_letter) {
                    passed = false;
                    break;
                }

                // if everything ok, move on to the next
                line[i] = ' ';
                line[i + 1] = ' ';
                i += 2;
            }
            if (passed)
                shader_string += line + '\n';

        } else {
            shader_string += line + '\n';
        }
    }
    file.close();

    //std::cout << shader_string << std::endl;

    return shader_string;
}

Shader ShaderGen::base_vertex_shader_gen(bool support_uv, bool support_normal) {
    std::string shader_code;
    if (support_uv && support_normal)
        shader_code = parse_shader_template("engine/shaders/templates/vertex_shader_template.glsl", "un");
    else if (support_normal)
        shader_code = parse_shader_template("engine/shaders/templates/vertex_shader_template.glsl", "n");
    else
        shader_code = parse_shader_template("engine/shaders/templates/vertex_shader_template.glsl", "u");

    return Shader{shader_code, GL_VERTEX_SHADER};
}


Shader ShaderGen::base_phong_shader_gen(bool support_uv) {
    std::string shader_code;
    if (support_uv)
        shader_code = parse_shader_template("engine/shaders/templates/phong.glsl", "un");
    else
        shader_code = parse_shader_template("engine/shaders/templates/phong.glsl", "n");

    return Shader{shader_code, GL_FRAGMENT_SHADER};
}

ShaderProgram ShaderGen::phong_shader_program_gen(bool has_uvs) {
    ShaderProgram sp {base_vertex_shader_gen(has_uvs, true), base_phong_shader_gen(has_uvs)};
    glUniformBlockBinding(sp.id, glGetUniformBlockIndex(sp.id, "MATRICES"), 0);
    return sp;
}




//GLuint64 handle = glGetTextureHandleARB(0);