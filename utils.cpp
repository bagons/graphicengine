#include <fstream>
#include <iostream>
#include <string>
#include <glad/glad.h>

unsigned int compile_shader_from_file(const char* file_path, GLenum shader_type) {
    unsigned int shader = glCreateShader(shader_type);
    std::cout << "loading shader file: " << file_path << std::endl;
    std::string shader_string;

    std::ifstream file(file_path);
    std::cout << "file_read_state: " << file.good() << std::endl;
    std::string line;
    while (std::getline(file, line)) {
        shader_string += line + '\n';
    }
    file.close();

    file_path = shader_string.c_str();
    glShaderSource(shader, 1, &file_path, nullptr);
    glCompileShader(shader);

    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    return shader;
}