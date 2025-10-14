#ifndef UTILS_H
#define UTILS_H
#pragma once
#include <glad/glad.h>

unsigned int setup_texture_from_file(const char* file_path, bool generate_minimaps=true);
unsigned int compile_shader_from_string(const char* shader_code, GLenum shader_type);

#endif //UTILS_H
