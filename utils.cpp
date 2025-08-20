#include <fstream>
#include <iostream>
#include <string>
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include "utils/stb_image.h"
#include "meshes.hpp"

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

unsigned int compile_shader_from_string(const char* shader_code, GLenum shader_type) {
    unsigned int shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, &shader_code, nullptr);
    glCompileShader(shader);

    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    return shader;
}

unsigned int setup_texture_from_file(const char* file_path, bool generate_minimaps) {
    unsigned int texture;
    glGenTextures(1, &texture);
    int width, height, channel_count;
    unsigned char* data = stbi_load(file_path, &width, &height, &channel_count, 0);

    if (data == nullptr) {
        std::cout << "Failed to load texture from file: " << file_path << std::endl;
    }

    glBindTexture(GL_TEXTURE_2D, texture);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load texture to gpu
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    if (generate_minimaps) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    stbi_image_free(data);
    return texture;
}