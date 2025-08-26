#include "meshes.hpp"

#include <filesystem>
#include <glad/glad.h>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <array>


void Mesh::load_mesh_to_gpu(const std::vector<float>* vertex_data, const std::vector<unsigned int>* indices, const bool has_texture_cords, const bool has_normals, const bool has_vertex_colors) {
    glGenBuffers(1, &vertex_buffer_object);
    glGenBuffers(1, &element_buffer_object);

    glGenVertexArrays(1, &vertex_array_object);
    glBindVertexArray(vertex_array_object);

    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
    glBufferData(GL_ARRAY_BUFFER, vertex_data->size() * sizeof(float), vertex_data->data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_object);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices->size() * sizeof(unsigned int), indices->data(), GL_STATIC_DRAW);
    vertex_count = indices->size();

    int stride = (3 + 3 * has_normals + 2 * has_texture_cords + 3 * has_vertex_colors) * sizeof(float);

    std::cout << "attrib stride: " << stride << " " << has_texture_cords << " " << has_normals << " " << has_vertex_colors << std::endl;

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, nullptr);
    glEnableVertexAttribArray(0);

    if (has_texture_cords) {
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3*sizeof(float)));
        glEnableVertexAttribArray(1);
    }

    if (has_normals) {
        glVertexAttribPointer(1 + has_texture_cords, 3, GL_FLOAT, GL_FALSE, stride, (void*)((3 + 2* has_texture_cords)*sizeof(float)));
        glEnableVertexAttribArray(1 + has_texture_cords);
    }

    if (has_vertex_colors) {
        glVertexAttribPointer(1 + has_normals + has_texture_cords, 3, GL_FLOAT, GL_FALSE, stride, (void*)((3 + 3 * has_normals + 2 * has_texture_cords)*sizeof(float)));
        glEnableVertexAttribArray(1 + has_normals + has_texture_cords);
    }

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
};


Mesh::Mesh(const std::vector<float>* vertex_data, const std::vector<unsigned int>* indices, const bool has_texture_cords, const bool has_normals, const bool has_vertex_colors) {
    load_mesh_to_gpu(vertex_data, indices, has_texture_cords, has_normals, has_vertex_colors);
}

Mesh::Mesh(){
    vertex_buffer_object = -1;
    vertex_array_object = -1;
    element_buffer_object = -1;
}

// OBJ PARSER STRUCTS FOR HASHMAP
struct UniqueVertexDataPoint {
    std::array<float, 8> vd = {0, 0, 0, 0, 0, 0, 0, 0};
    size_t i = 0, j = 0, k = 0;

    bool operator==(const UniqueVertexDataPoint& other) const {
        return i == other.i && j == other.j && k == other.k;
    }
};

struct UniqueVertexDataPointHash {
    size_t operator()(const UniqueVertexDataPoint& key) const {
        return std::hash<int>()(key.i) ^ (std::hash<int>()(key.j) << 8) ^ (std::hash<int>()(key.k) << 16);
        /*return std::hash<float>()(key.vd[0]) ^
                std::hash<float>()(key.vd[1]) ^
                    std::hash<float>()(key.vd[2]) ^
                        std::hash<float>()(key.vd[3]) ^
                            std::hash<float>()(key.vd[4]) ^
                                std::hash<float>()(key.vd[5]) ^
                                    std::hash<float>()(key.vd[6]) ^
                                        std::hash<float>()(key.vd[7]);
        */
    }
};


Mesh::Mesh(const char* file_path) {
    vertex_buffer_object = -1;
    vertex_array_object = -1;
    element_buffer_object = -1;

    std::ifstream file(file_path);
    std::cout << "file state: " << file.good() << std::endl;
    std::string line;

    std::vector<float> vertex_data_vec[3];
    std::vector<size_t> vertex_triplets;

    bool has_texture_cords = false;
    bool has_normals = false;

    // PARSING OF .OBJ FILE
    // go line by line
    // TODO seems a bit slow, after parsing unordered map is instant, this will be the constraint
    while (std::getline(file, line)) {
        // if v data
        if (line[0] == 'v') {
            char data_type = 0;
            if (line[1] == 't') {
                data_type = 1;
                has_texture_cords = true;
            } else if (line[1] == 'n') {
                data_type = 2;
                has_normals = true;
            }

            // get only a slice of line (only the 3 floats in text)
            const char * l = line.c_str() + 2;
            // fast parse
            char * end;
            vertex_data_vec[data_type].push_back(std::strtof(l, &end));
            vertex_data_vec[data_type].push_back(std::strtof(end, &end));
            vertex_data_vec[data_type].push_back(std::strtof(end, nullptr));
        }
        // if face definition
        else if (line[0] == 'f') {
            const char *l = line.c_str() + 2;
            char * end = const_cast<char *>(l);
            // fast parse of x/y/z (because of '/' we have an offset in the pointer)
            for (int i = 0; i < 3; i++) {
                vertex_triplets.push_back(std::strtol(end, &end, 10));

                if (has_normals || has_texture_cords)
                    vertex_triplets.push_back(std::strtol(end + 1, &end, 10));
                if (has_normals && has_texture_cords)
                    vertex_triplets.push_back(std::strtol(end + 1, &end, 10));
            }
        }
    }

    unsigned int vertex_data_group_size = 1 + has_normals + has_texture_cords;

    std::cout << ".obj parsed!" << std::endl;

    std::unordered_map<UniqueVertexDataPoint, unsigned int, UniqueVertexDataPointHash> unique_vertex_data_points;

    std::vector<float> vertex_data;
    std::vector<unsigned int> indices;

    unsigned int indecy_count = 0;

    for (size_t i = 0; i < (vertex_triplets.size() / vertex_data_group_size); i++) {
        // create UniqueVertexDataPoint object
        UniqueVertexDataPoint vdp{};

        vdp.i = vertex_triplets[i * vertex_data_group_size] - 1;
        vdp.vd[0] = vertex_data_vec[0][vdp.i * 3];
        vdp.vd[1] = vertex_data_vec[0][vdp.i * 3 + 1];
        vdp.vd[2] = vertex_data_vec[0][vdp.i * 3 + 2];


        // fill in data of if they exist
        size_t t = 3;

        if (has_texture_cords) {

            vdp.j = vertex_triplets[i * vertex_data_group_size + 1] - 1;

            vdp.vd[t] = vertex_data_vec[1][vdp.j * 2];
            vdp.vd[t + 1] = vertex_data_vec[1][vdp.j * 2 + 1];
            t = 5;
        }

        if (has_normals) {
            vdp.k = vertex_triplets[i * vertex_data_group_size + 1 + has_texture_cords] - 1;

            vdp.vd[t] = vertex_data_vec[2][vdp.k * 3];
            vdp.vd[t + 1] = vertex_data_vec[2][vdp.k * 3 + 1];
            vdp.vd[t + 2] = vertex_data_vec[2][vdp.k * 3 + 2];
        }


        // check if already exists with hashmap
        auto match = unique_vertex_data_points.find(vdp);
        if (match == unique_vertex_data_points.end()) {
            // add new indecy
            unique_vertex_data_points[vdp] = indecy_count;
            indices.push_back(indecy_count);
            indecy_count += 1;

            // insert vertex data
            vertex_data.insert(vertex_data.end(), vdp.vd.begin(), vdp.vd.begin() + 3 + 3 * has_normals + 2 * has_texture_cords);
        } else {
            indices.push_back(match->second);
        }
    }

    std::cout << "vertex_data size: " << vertex_data.size() << std::endl;
    std::cout << "indecies size: " << indices.size() << std::endl;

    // set correct flags based on .obj properties. 1/1/1 triplets v/vt/vn
    load_mesh_to_gpu(&vertex_data, &indices, has_texture_cords, has_texture_cords, false);
}

Mesh::~Mesh() {
    glDeleteVertexArrays(1,  &vertex_array_object);
    glDeleteBuffers(1, &vertex_buffer_object);
    glDeleteBuffers(1, &element_buffer_object);
}

Meshes::Meshes() {

}

Meshes::~Meshes() {
    if (cube != nullptr)
        delete cube;

    if (plane != nullptr)
        delete plane;
}

Mesh* Meshes::get_plane() {
    if (plane == nullptr)
        plane = new Mesh{&PLANE_VERTEX_DATA, &PLANE_INDICES, true, true};
    return plane;
}
