#ifndef UTILS_H
#define UTILS_H
#pragma once
#include <glad/glad.h>

unsigned int compile_shader_from_file(const char* file_path, GLenum shader_type);
unsigned int setup_texture_from_file(const char* file_path, bool generate_minimaps=true);

#endif //UTILS_H
