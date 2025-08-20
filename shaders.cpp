#include "shaders.hpp"
#include <iostream>
#include <fstream>
#include "utils.h"


ShaderProgram::ShaderProgram(unsigned int vertex_shader, unsigned int fragment_shader) {
    id = glCreateProgram();
    glAttachShader(id, vertex_shader);
    glAttachShader(id, fragment_shader);
    glLinkProgram(id);
}


ShaderProgram::ShaderProgram(unsigned int vertex_shader, unsigned int fragment_shader, const std::vector<const char *>& quick_uniforms) {
    id = glCreateProgram();
    glAttachShader(id, vertex_shader);
    glAttachShader(id, fragment_shader);
    glLinkProgram(id);

    for (const auto& uniform : quick_uniforms) {
        quick_uniform_ids.push_back(glGetUniformLocation(id, uniform));
    }
}


void ShaderProgram::use() {
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
                std::cout << line[i] << std::endl;

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

    std::cout << shader_string << std::endl;

    return shader_string;
}

unsigned int ShaderGen::base_vertex_shader_gen(bool support_uv, bool support_normal) {
    std::string shader_code = "";
    if (support_uv && support_normal)
        shader_code = parse_shader_template("engine/shaders/templates/vertex_shader_template.glsl", "un");
    else if (support_normal)
        shader_code = parse_shader_template("engine/shaders/templates/vertex_shader_template.glsl", "n");
    else
        shader_code = parse_shader_template("engine/shaders/templates/vertex_shader_template.glsl", "u");


    return compile_shader_from_string(shader_code.c_str(), GL_VERTEX_SHADER);
}


unsigned int ShaderGen::base_phong_shader_gen(bool support_uv) {
    std::string shader_code = "";
    if (support_uv)
        shader_code = parse_shader_template("engine/shaders/templates/phong.glsl", "un");
    else
        shader_code = parse_shader_template("engine/shaders/templates/phong.glsl", "n");

    return compile_shader_from_string(shader_code.c_str(), GL_FRAGMENT_SHADER);
}

ShaderProgram ShaderGen::phong_shader_program_gen(bool has_uvs) {
    const std::vector quick_uniforms = {
        "object_color"
    };
    return ShaderProgram{base_vertex_shader_gen(has_uvs, true), base_phong_shader_gen(has_uvs), quick_uniforms};
}




//GLuint64 handle = glGetTextureHandleARB(0);