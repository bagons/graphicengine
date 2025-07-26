#ifndef MESHES_HPP
#define MESHES_HPP

#include <vector>

#include "things.hpp"

class Meshes {
public:
    const std::vector<float> CUBE_VERTICES = {
        -0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        0.5f, 0.5f, -0.5f,

        -0.5f, -0.5f, 0.5f,
        0.5f, -0.5f, 0.5f,
        -0.5f,  0.5f, 0.5f,
        0.5f, 0.5f, 0.5f,
    };

    const std::vector<unsigned int> CUBE_INDICES = {
        0, 1, 2,
        1, 3, 2,
        6, 7, 3,
        3, 2, 6,
        4, 5, 6,
        5, 7, 6
    };

    Mesh* cube;
    Meshes();
    Mesh* get_cube();
    ~Meshes();
};

#endif //MESHES_HPP
