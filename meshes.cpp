#include "meshes.hpp"
#include <glad/glad.h>
#include <fstream>
#include <iostream>
#include <string>


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

Mesh::Mesh(const char* file_path) {
    vertex_buffer_object = -1;
    vertex_array_object = -1;
    element_buffer_object = -1;

    std::ifstream file(file_path);
    std::string line;

    std::vector<float> vertex_data_vec[3];
    std::vector<int *> vertex_triplets;

    unsigned int v_line_data_stride = 0;
    bool got_vertex_data_stride = false;

    // PARSING OF .OBJ FILE
    // go line by line
    while (std::getline(file, line)) {
        // keep a buffer when scanning line
        std::string buf;
        // know what data is the line talking about
        char data_type = -1;
        // go through all chars in string
        for (size_t i = 0; i < line.size(); i++) {
            // if there is a space or it is the end of the line, check what have we read in the buffer
            if ((line[i] == ' ' || i == line.size() - 1) && buf.size() > 0) {
                // if there is no data type defined, buffer will have data type specificaiton
                if (data_type == -1) {
                    if (buf == "v")
                        data_type = 0;
                    else if (buf == "vt")
                        data_type = 1;
                    else if (buf == "vn")
                        data_type = 2;
                    else if (buf == "f")
                        data_type = 3;
                } else if (data_type == 3) {
                    // special behaviour for parsing indecies
                    // add the last character if the end to the line
                    if (i == line.size() - 1) buf += line[i];
                    // indecy combination (up to 3 number) (v/vt/vn) <=> (1/4/1)
                    auto vertex_triplet = static_cast<int *>(calloc(3, sizeof(int)));
                    int num_idx = 0;
                    char data_switch = 0;
                    // go from end to start and read the int, by increasing exponent
                    for (size_t x = buf.size(); x > 0; x--) {
                        std::cout << buf[x - 1] << " ";
                        if (buf[x - 1] == '/') {
                            data_switch += 1;
                            num_idx = 0;
                            continue;
                        }
                        vertex_triplet[data_switch] += (buf[x - 1] - '0') * pow(10, num_idx);
                        num_idx++;
                    }
                    std::cout << std::endl;
                    // once the number was read, offset it by one (.obj index from 1), then add to indecies list
                    vertex_triplets.push_back(vertex_triplet);
                } else {
                    if (data_type == 0 && !got_vertex_data_stride) v_line_data_stride++;
                    // add the last character if the end to the line
                    if (i == line.size() - 1) {
                        buf += line[i];
                        // add 100 to specify that there is no need to update vertex_data_stride any more
                        got_vertex_data_stride = true;
                    }
                    vertex_data_vec[data_type].push_back(std::stof(buf));
                }
                // reset buffer after each space or line end
                buf = "";
            } else { // else add another character to the buffer
                buf += line[i];
            }
        }
    }

    std::cout << ".obj parsed!" << std::endl;

    const int hashmap_sizes[13] = {17, 31, 53, 97, 193, 389, 769, 1009, 2003, 4001, 8009, 16001, 32003};
    int hashmap_size = vertex_triplets.size();

    for (int i = 0; i < 13; i++) {
        if (hashmap_sizes[i] > hashmap_size) {
            hashmap_size = hashmap_sizes[i];
            break;
        }
    }

    auto vertex_hashmap = static_cast<int *>(calloc(hashmap_size, sizeof(int)));

    std::vector<float> vertex_data;
    std::vector<unsigned int> indices;

    size_t vertex_data_stride = 0;

    size_t indecy_data_idx = 1 + (vertex_triplets[0][2] != 0) * 1;
    // wtf how did i get here (if idecy_data == 0 -> + 1, if == 1 -> 2, if == 2 -> 5)
    vertex_data_stride = v_line_data_stride + round(indecy_data_idx * 2.3f);

    if (vertex_hashmap == nullptr) {
        std::cout << "loading mesh file - null pointer has been returned";
    } else {
        std::cout << vertex_triplets.size() << " <- indices size" << std::endl;

        std::cout << "v line data stride + other shit -> " << v_line_data_stride << " " << round(indecy_data_idx * 2.3f) << std::endl;
        //std::cout << indecy_data_idx << " <- idi " << vertex_triplets[0][0] << " " << vertex_triplets[0][1] << " " << vertex_triplets[0][2] << std::endl;
        for (auto vertex_triplet : vertex_triplets) {
            // calculate hash for vertex with vertex position
            int vertex_index = vertex_triplet[indecy_data_idx] - 1;
            std::cout << vertex_data_vec[0].size() << " <- size " << vertex_index << " " << vertex_data_stride << std::endl;

            float vx = vertex_data_vec[0][vertex_index * v_line_data_stride];
            float vy = vertex_data_vec[0][vertex_index * v_line_data_stride + 1];
            float vz = vertex_data_vec[0][vertex_index * v_line_data_stride + 2];

            // optional p
            float vtx = 0;
            float vty = 0;

            float vnx = 0;
            float vny = 0;
            float vnz = 0;

            size_t thehash =
            std::hash<float>()(vx) ^
                std::hash<float>()(vy) ^
                    std::hash<float>()(vz);

            // calculate hash for vertex with texture coordinates
            if (indecy_data_idx > 0) {
                // get correct texture coordinate based on .obj face specification (indecy)
                int texture_coord_index = vertex_triplet[indecy_data_idx - 1] - 1;
                //std::cout << "texture coord: " << texture_coord_index << " " << vertex_data_vec[1].size() << std::endl;

                vtx = vertex_data_vec[1][texture_coord_index * 2];
                vty = vertex_data_vec[1][texture_coord_index * 2 + 1];
                // calculate hash
                thehash ^=
                       std::hash<float>()(vtx) ^
                           std::hash<float>()(vty);
            }

            // calculate hash for vertex with normals
            if (indecy_data_idx > 1) {
                // get correct normal based on .obj face specification (indecy)
                int normal_index = vertex_triplet[indecy_data_idx - 2] - 1;
                //std::cout << "normal: " << normal_index << " " << vertex_data_vec[2].size() << std::endl;

                vnx = vertex_data_vec[2][normal_index * 3];
                vny = vertex_data_vec[2][normal_index * 3 + 1];
                vnz = vertex_data_vec[2][normal_index * 3 + 2];

                // calculate hash
                thehash ^=
                    std::hash<float>()(vnx) ^
                        std::hash<float>()(vny) ^
                            std::hash<float>()(vnz);
            }

            size_t hashmap_idx = thehash % hashmap_size;

            bool found_match = false;

            // if hash conflict
            while (vertex_hashmap[hashmap_idx] > 0) {
                size_t conflict_idx = vertex_hashmap[hashmap_idx] - 1;

                // if duplicate match, just add hashmap idx
                if (vertex_triplet[0] == vertex_triplets[conflict_idx][0] && vertex_triplet[1] == vertex_triplets[conflict_idx][1] && vertex_triplet[2] == vertex_triplets[conflict_idx][2]) {
                    indices.push_back(vertex_hashmap[hashmap_idx] - 1);
                    found_match = true;
                    break;
                }
                // else continue util we find a match, or an emtpy slot

                hashmap_idx++;

                if (hashmap_idx == hashmap_size)
                    hashmap_idx = 0;
            }
            //std::cout << "possible conflict: " << vertex_triplet[0] << " " << vertex_triplet[1] << " " << vertex_triplet[2] << " vs. " << vertex_triplets[conflict_idx][0] << " " << vertex_triplets[conflict_idx][1] << " " << vertex_triplets[conflict_idx][2] << std::endl;

            // if we found a match, we continue to the next vertex triplet
            if (found_match) continue;
            // if we found a empty slot, we add a new indecy

            // add new vertex with an indecy
            indices.push_back(vertex_data.size() / vertex_data_stride);
            vertex_hashmap[hashmap_idx] = vertex_data.size() / vertex_data_stride + 1;

            vertex_data.push_back(vx);
            vertex_data.push_back(vy);
            vertex_data.push_back(vz);

            // add texture coord data if needed
            if (indecy_data_idx > 0) {
                vertex_data.push_back(vtx);
                vertex_data.push_back(vty);
            }

            // add normal data if needed
            if (indecy_data_idx > 1) {
                vertex_data.push_back(vnx);
                vertex_data.push_back(vny);
                vertex_data.push_back(vnz);
            }
        }
    }

    for (auto vertex_triplet : vertex_triplets)
        free(vertex_triplet);

    free(vertex_hashmap);

    std::cout << "indecies:" << std::endl;
    for (auto i : indices)
        std::cout << i << std::endl;


    std::cout << "vd stride:" << vertex_data_stride << std::endl;

    for (int i = 0; i < vertex_data.size() / vertex_data_stride; i++) {
        std::cout << vertex_data[i * vertex_data_stride] << " ";
        std::cout << vertex_data[i * vertex_data_stride + 1] << " ";
        std::cout << vertex_data[i * vertex_data_stride + 2] << " ";
        std::cout << vertex_data[i * vertex_data_stride + 3] << " ";
        std::cout << vertex_data[i * vertex_data_stride + 4] << " ";
        std::cout << vertex_data[i * vertex_data_stride + 5] << " ";
        std::cout << vertex_data[i * vertex_data_stride + 6] << " ";
        std::cout << vertex_data[i * vertex_data_stride + 7] << " " << std::endl;
    }

    // set correct flags based on .obj properties. 1/1/1 triplets v/vt/vn
    load_mesh_to_gpu(&vertex_data, &indices, indecy_data_idx > 0, indecy_data_idx > 1, v_line_data_stride > 3);
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
