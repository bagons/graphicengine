#include "textures.hpp"
#include "utils.h"
#include <iostream>

Texture *Textures::load_texture(const char *file_path, bool generate_minimap) {
    unsigned int texture_id = setup_texture_from_file(file_path, generate_minimap);

    const GLuint64 handle = glGetTextureHandleARB(texture_id);
    glMakeTextureHandleResidentARB(handle);

    if (!glIsTextureHandleResidentARB(handle)) {
        std::cout << "Texture handle is NOT resident!" << std::endl;
    } else {
        std::cout << "Texture handle is resident!" << std::endl;
    }

    auto tex = new Texture{};
    tex->id = texture_id;
    tex->handle = handle;

    textures.push_back(tex);

    return tex;
}

Textures::~Textures() {
    for (auto tex : textures) {
        delete tex;
    }
}