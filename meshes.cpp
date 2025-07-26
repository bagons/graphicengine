#include "meshes.hpp"

Meshes::Meshes() {

}

Meshes::~Meshes() {
    if (cube != nullptr) {
        delete cube;
    }
}

Mesh* Meshes::get_cube() {
    if (cube == nullptr) {
        cube = new Mesh{CUBE_VERTICES, CUBE_INDICES};
    }
    return cube;
}