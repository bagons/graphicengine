#include "meshes.hpp"
#include "graphicengine.hpp"

#include <filesystem>
#include <glad/glad.h>
#include <fstream>
#include <iostream>
#include <string>
#include <array>


void Mesh::load_mesh_to_gpu(const std::vector<float>* vertex_data, const std::vector<unsigned int>* indices, const bool has_uvs, const bool has_normals, const bool has_vertex_colors) {
    glGenBuffers(1, &vertex_buffer_object);
    glGenBuffers(1, &element_buffer_object);

    glGenVertexArrays(1, &vertex_array_object);
    glBindVertexArray(vertex_array_object);

    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
    glBufferData(GL_ARRAY_BUFFER, vertex_data->size() * sizeof(float), vertex_data->data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_object);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices->size() * sizeof(unsigned int), indices->data(), GL_STATIC_DRAW);
    vertex_count = indices->size();

    int stride = (3 + 3 * has_normals + 2 * has_uvs + 3 * has_vertex_colors) * sizeof(float);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, nullptr);
    glEnableVertexAttribArray(0);

    if (has_uvs) {
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3*sizeof(float)));
        glEnableVertexAttribArray(1);
    }

    if (has_normals) {
        glVertexAttribPointer(1 + has_uvs, 3, GL_FLOAT, GL_FALSE, stride, (void*)((3 + 2* has_uvs)*sizeof(float)));
        glEnableVertexAttribArray(1 + has_uvs);
    }

    if (has_vertex_colors) {
        glVertexAttribPointer(1 + has_normals + has_uvs, 3, GL_FLOAT, GL_FALSE, stride, (void*)((3 + 3 * has_normals + 2 * has_uvs)*sizeof(float)));
        glEnableVertexAttribArray(1 + has_normals + has_uvs);
    }

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
};


Mesh::Mesh(const std::vector<float>* vertex_data, const std::vector<unsigned int>* indices, const bool has_uvs, const bool has_normals, const bool has_vertex_colors) : has_uvs(has_uvs), has_normals(has_normals) {
    load_mesh_to_gpu(vertex_data, indices, has_uvs, has_normals, has_vertex_colors);
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
        return std::hash<size_t>()(key.i) ^ (std::hash<size_t>()(key.j) << 8) ^ (std::hash<size_t>()(key.k) << 16);
    }
};

std::string after_char(const std::string& s, char delimiter) {
    auto pos = s.find(delimiter);
    return s.substr(pos + 1);
}

std::string normalize_path(std::string path) {
    std::ranges::replace(path, '\\', '/');
    return path;
}


std::unordered_map<std::string, std::shared_ptr<Material>> parse_mlt_file(const char* file_path, bool has_normals, bool has_uvs) {
    std::unordered_map<std::string, std::shared_ptr<Material>> materials;

    std::ifstream file(file_path);
    std::cout << "PARSING MLT" << std::endl;
    std::cout << "file state: " << file.good() << std::endl;
    std::string line;

    // parsed values
    std::shared_ptr<Material> mat = nullptr;

    auto template_shader_program = ge.shaders.get_base_material(has_uvs, has_normals)->shader_program;

    while (std::getline(file, line)) {
        if (line[0] == 'n') {
            auto material_name = after_char(line, ' ');
            std::cout << "new mat "<< material_name << std::endl;
            mat = std::make_shared<Material>(template_shader_program);
            materials[material_name] = mat;
        } else if (line[0] == '\0') {
            mat = nullptr;
        }

        //
        if (mat != nullptr) {
            const char * l = line.c_str() + 3;

            if (line[1] == 'K') {
                char * end;
                auto v3 = glm::vec3{};
                v3.x = std::strtof(l, &end);
                v3.y = std::strtof(end, &end);
                v3.z = std::strtof(end, nullptr);

                if (line[2] == 'a' and has_normals)
                    mat->save_uniform_value("material.ambient", v3);
                else if (line[2] == 'd')
                    mat->save_uniform_value("material.diffuse", v3);
                else if (line[2] == 's' and has_normals)
                    mat->save_uniform_value("material.specular", v3);
            } else if (line[1] == 'N' and has_normals) {
                if (line[2] == 'i')
                    mat->save_uniform_value("material.shininess", std::strtof(l, nullptr));
            } else if (line[1] == 'm') {
                if (line[6] == 'd') {
                    mat->save_uniform_value("material.has_albedo", true);
                    auto texture_path = std::filesystem::path(file_path).parent_path() / normalize_path(after_char(line, ' '));
                    std::cout << "texture path: " << texture_path << std::endl;
                    auto texture_ref = ge.textures.load(texture_path.string().c_str());
                    mat->save_uniform_value("material.albedo_texture", texture_ref);
                }
            }
        }
    }

    return materials;
}


// PARSES OBJ FILE AS A MODEL (separating vertex groups, parsing materials from mtllib)
void parse_obj_file(const char* file_path, std::vector<float> (&vertex_data_vec)[3], std::vector<std::vector<size_t>>& vertex_groups, std::vector<std::shared_ptr<Material>>& materials, bool &has_uvs, bool &has_normals) {
    std::ifstream file(file_path);
    std::cout << "file state: " << file.good() << std::endl;
    std::string line;

    vertex_groups.push_back(std::vector<size_t>());
    int current_group_idx = 0;

    has_normals = false;
    has_uvs = false;

    materials.push_back(nullptr);

    std::unordered_map<std::string, std::shared_ptr<Material>> mtl_materials;

    std::string current_material_name;

    // PARSING OF .OBJ FILE
    // go line by line
    // TODO seems a bit slow, after parsing unordered map is instant, this will be the constraint
    while (std::getline(file, line)) {
        // mtllib X
        if (line[0] == 'm') {
            auto mlt_file = normalize_path(after_char(line, ' '));
            std::filesystem::path path(file_path);
            std::filesystem::path mtl_path = path.parent_path() / mlt_file;
            mtl_materials = parse_mlt_file(mtl_path.string().c_str(), true, true); // TODO NOT FINISHED
        }
        // if v data
        else if (line[0] == 'v') {
            char data_type = 0;
            if (line[1] == 't') {
                data_type = 1;
                has_uvs = true;
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
                // read first f
                vertex_groups[current_group_idx].push_back(std::strtol(end, &end, 10));

                if (has_normals || has_uvs)
                    vertex_groups[current_group_idx].push_back(std::strtol(end + 1, &end, 10));
                if (has_normals && has_uvs)
                    vertex_groups[current_group_idx].push_back(std::strtol(end + 1, &end, 10));
            }

            // IF THERE IS A FORTH POINT IN A FACE (A QUAD)
            if (*end != '\0' and *(end + 1) != '\0') {
                int vertex_data_group_size = 1 + has_normals + has_uvs;
                size_t vtx_group_list_end = vertex_groups[current_group_idx].size() - 1;
                // spawn second triangle

                // fill in points to form a triangle
                // point 1
                vertex_groups[current_group_idx].push_back(
                    vertex_groups[current_group_idx][ vtx_group_list_end - 3 * vertex_data_group_size + 1]
                    );

                if (has_normals || has_uvs) {
                    vertex_groups[current_group_idx].push_back(
                        vertex_groups[current_group_idx][vtx_group_list_end - 3 * vertex_data_group_size + 2]
                    );
                }

                if (has_normals && has_uvs) {
                    vertex_groups[current_group_idx].push_back(
                        vertex_groups[current_group_idx][vtx_group_list_end - 3 * vertex_data_group_size + 3]
                    );
                }

                //point 2
                vertex_groups[current_group_idx].push_back(
                    vertex_groups[current_group_idx][vtx_group_list_end - 1 * vertex_data_group_size + 1]
                    );

                if (has_normals || has_uvs) {
                    vertex_groups[current_group_idx].push_back(
                        vertex_groups[current_group_idx][vtx_group_list_end - 1 * vertex_data_group_size + 2]
                    );
                }

                if (has_normals && has_uvs) {
                    vertex_groups[current_group_idx].push_back(
                        vertex_groups[current_group_idx][vtx_group_list_end - 1 * vertex_data_group_size + 3]
                    );
                }

                //parse NEW point
                int dp_idx = std::strtol(end + 1, &end, 10);
                vertex_groups[current_group_idx].push_back(dp_idx);

                if (has_normals || has_uvs) {
                    dp_idx = std::strtol(end + 1, &end, 10);
                    vertex_groups[current_group_idx].push_back(dp_idx);
                }
                if (has_normals && has_uvs) {
                    dp_idx = std::strtol(end + 1, &end, 10);
                    vertex_groups[current_group_idx].push_back(dp_idx);
                }
            }
        }
        // usemtl X
        else if (line[0] == 'u') {
            current_material_name = after_char(line, ' ');
            materials[materials.size() - 1] = mtl_materials[current_material_name];
        }
        // groups
        else if (line[0] == 'g') {
            // add new material slot
            if (current_material_name.empty())
                materials.push_back(nullptr);
            else
                materials.push_back(mtl_materials[current_material_name]);

            // create
            vertex_groups.emplace_back();
            current_group_idx += 1;
        }
    }
}


// PARSE OBJ FILE AS A UNIFORM MESH WITH NO MATERIAL
void parse_obj_file(const char* file_path, std::vector<float> (&vertex_data_vec)[3], std::vector<size_t>& vertex_group, bool &has_uvs, bool &has_normals) {
    std::ifstream file(file_path);
    std::cout << "file state: " << file.good() << std::endl;
    std::string line;
    has_normals = false;
    has_uvs = false;

    // PARSING OF .OBJ FILE
    // go line by line
    // TODO seems a bit slow, after parsing unordered map is instant, this will be the constraint
    while (std::getline(file, line)) {
        // if v data
        if (line[0] == 'v') {
            char data_type = 0;
            if (line[1] == 't') {
                data_type = 1;
                has_uvs = true;
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
                vertex_group.push_back(std::strtol(end, &end, 10));

                if (has_normals || has_uvs)
                    vertex_group.push_back(std::strtol(end + 1, &end, 10));
                if (has_normals && has_uvs)
                    vertex_group.push_back(std::strtol(end + 1, &end, 10));

            }
            // IF THERE IS A FORTH POINT IN A FACE (A QUAD)
            if (*end != '\0' and *(end + 1) != '\0') {
                int vertex_data_group_size = 1 + has_normals + has_uvs;

                size_t vtx_group_list_end = vertex_group.size() - 1;
                // spawn second triangle
                // fill in points to form a triangle
                // point 1
               vertex_group.push_back(
                    vertex_group[ vtx_group_list_end - 3 * vertex_data_group_size + 1]
                    );

                if (has_normals || has_uvs) {
                    vertex_group.push_back(
                        vertex_group[vtx_group_list_end - 3 * vertex_data_group_size + 2]
                    );
                }

                if (has_normals && has_uvs) {
                    vertex_group.push_back(
                        vertex_group[vtx_group_list_end - 3 * vertex_data_group_size + 3]
                    );
                }

                //point 2
                vertex_group.push_back(
                    vertex_group[vtx_group_list_end - 1 * vertex_data_group_size + 1]
                    );

                if (has_normals || has_uvs) {
                    vertex_group.push_back(
                        vertex_group[vtx_group_list_end - 1 * vertex_data_group_size + 2]
                    );
                }

                if (has_normals && has_uvs) {
                    vertex_group.push_back(
                        vertex_group[vtx_group_list_end - 1 * vertex_data_group_size + 3]
                    );
                }

                //parse NEW point
                int dp_idx = std::strtol(end + 1, &end, 10);
                vertex_group.push_back(dp_idx);

                if (has_normals || has_uvs) {
                    dp_idx = std::strtol(end + 1, &end, 10);
                    vertex_group.push_back(dp_idx);
                }
                if (has_normals && has_uvs) {
                    dp_idx = std::strtol(end + 1, &end, 10);
                    vertex_group.push_back(dp_idx);
                }
            }
        }
    }
}


void construct_mesh_data_from_parsed_obj_data(const std::vector<float> (&vertex_data_vec)[3], const std::vector<size_t>& vertex_triplets, const bool has_normals, const bool has_texture_cords, std::vector<float>& out_vertex_data, std::vector<unsigned int>& out_indices) {
    std::unordered_map<UniqueVertexDataPoint, unsigned int, UniqueVertexDataPointHash> unique_vertex_data_points;

    unsigned int vertex_data_group_size = 1 + has_normals + has_texture_cords;

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
                out_indices.push_back(indecy_count);
                indecy_count += 1;

                // insert vertex data
                out_vertex_data.insert(out_vertex_data.end(), vdp.vd.begin(), vdp.vd.begin() + 3 + 3 * has_normals + 2 * has_texture_cords);
            } else {
                out_indices.push_back(match->second);
            }
        }

        std::cout << "vertex_data size: " << out_vertex_data.size() << std::endl;
        std::cout << "indecies size: " << out_indices.size() << std::endl;
}


Mesh::Mesh(const char* file_path) {
    vertex_buffer_object = -1;
    vertex_array_object = -1;
    element_buffer_object = -1;

    // define structures
    std::vector<float> vertex_data_vec[3];
    std::vector<size_t> vertex_group;

    // use structures to parse .obj file
    parse_obj_file(file_path, vertex_data_vec, vertex_group, has_uvs, has_normals);

    std::cout << ".obj parsed!" << std::endl;

    // define new structures, for reordering
    std::vector<float> vertex_data;
    std::vector<unsigned int> indices;

    // use structures to create correctly formated values for Mesh

    // vertex_groups[0], because usually we would loop through all the groups and create the appropriate Mesh
    // but this is just a mesh, so we merged_all_groups, and now we work with only one group
    construct_mesh_data_from_parsed_obj_data(vertex_data_vec, vertex_group, has_normals, has_uvs, vertex_data, indices);
    load_mesh_to_gpu(&vertex_data, &indices, has_uvs, has_normals, false);
}

Model::Model(const char* file_path) {
    // define structures
    std::vector<float> vertex_data_vec[3];
    std::vector<std::vector<size_t>> vertex_groups;

    // use structures to parse .obj file
    parse_obj_file(file_path, vertex_data_vec, vertex_groups, materials, has_uvs, has_normals);

    std::cout << ".obj parsed!" << std::endl;

    for (auto &vertex_group : vertex_groups) {

        // define new structures, for reordering
        std::vector<float> vertex_data;
        std::vector<unsigned int> indices;


        // use structures to create correctly formated values for Mesh
        construct_mesh_data_from_parsed_obj_data(vertex_data_vec, vertex_group, has_normals, has_uvs, vertex_data, indices);

        auto msh = std::make_shared<Mesh>(&vertex_data, &indices, has_uvs, has_normals, false);
        meshes.push_back(msh);
    }

    std::cout << "post model" << std::endl;
}

Mesh::~Mesh() {
    glDeleteVertexArrays(1,  &vertex_array_object);
    glDeleteBuffers(1, &vertex_buffer_object);
    glDeleteBuffers(1, &element_buffer_object);
}

Meshes::Meshes() {
    //plane = std::make_shared<Mesh>(&PLANE_VERTEX_DATA, &PLANE_INDICES, true, true);
}


