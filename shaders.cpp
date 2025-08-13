#include "shaders.hpp"

#include <iostream>
#include <__msvc_ostream.hpp>

#include "gtc/type_ptr.inl"


ShaderProgram::ShaderProgram(unsigned int vertex_shader, unsigned int fragment_shader, const std::span<const char *> quick_uniforms) {
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
    unsigned int uniform_loc = glGetUniformLocation(id, uniform_name);
    std::cout << "uniform loc: " << uniform_loc << std::endl;
    glUniformHandleui64ARB(uniform_loc, texture_handle);
}
void ShaderProgram::set_uniform(const char* uniform_name, float val) const {
    glUniform1f(glGetUniformLocation(id, uniform_name), val);
}

void ShaderProgram::set_uniform(const char* uniform_name, glm::vec3 val) const {
    glUniform3fv(glGetUniformLocation(id, uniform_name), 1, glm::value_ptr(val));
}


//GLuint64 handle = glGetTextureHandleARB(0);