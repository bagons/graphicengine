#include "meshes.hpp"
#include "graphicengine.hpp"

#include <filesystem>
#include <glad/glad.h>
#include <fstream>
#include <iostream>
#include <string>
#include <array>
#include <algorithm>


void Mesh::load_mesh_to_gpu(const std::vector<float>* vertex_data, const std::vector<unsigned int>* indices, const bool has_uvs, const bool has_normals, const bool has_tangents, const bool has_vertex_colors) {
    glGenBuffers(1, &vertex_buffer_object);
    glGenBuffers(1, &element_buffer_object);

    glGenVertexArrays(1, &vertex_array_object);
    glBindVertexArray(vertex_array_object);

    vertex_count = static_cast<int>(indices->size());

    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
    glBufferData(GL_ARRAY_BUFFER, static_cast<int>(vertex_data->size()) * static_cast<int>(sizeof(float)), vertex_data->data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_object);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertex_count * static_cast<int>(sizeof(unsigned int)), indices->data(), GL_STATIC_DRAW);

    const int stride = (3 + (has_normals ? 3 : 0) + (has_uvs ? 2 : 0) + (has_tangents ? 3 : 0) + (has_vertex_colors ? 3 : 0)) * static_cast<int>(sizeof(float));

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, nullptr);
    glEnableVertexAttribArray(0);

    if (has_uvs) {
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void *>(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }

    if (has_normals) {
        glVertexAttribPointer(1 + has_uvs, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void *>((3 + 2 * has_uvs) * sizeof(float)));
        glEnableVertexAttribArray(1 + has_uvs);
    }

    if (has_tangents) {
        glVertexAttribPointer(1 + has_normals + has_uvs, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void *>((3 + 3 * has_normals + 2 * has_uvs) * sizeof(float)));
        glEnableVertexAttribArray(1 + has_normals + has_uvs);
    }

    if (has_vertex_colors) {
        glVertexAttribPointer(1 + has_normals + has_uvs, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void *>((3 + 3 * has_normals + 3 * has_tangents + 2 * has_uvs) * sizeof(float)));
        glEnableVertexAttribArray(1 + has_normals + has_uvs);
    }

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterward we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
};


Mesh::Mesh(const std::vector<float>* vertex_data, const std::vector<unsigned int>* indices, const bool has_uvs, const bool has_normals, const bool has_tangents, const bool has_vertex_colors) : has_uvs(has_uvs), has_normals(has_normals), has_tangents(has_tangents) {
    load_mesh_to_gpu(vertex_data, indices, has_uvs, has_normals, has_tangents, has_vertex_colors);
}

unsigned int Mesh::get_vertex_array_object() const {
    return vertex_array_object;
}

int Mesh::get_vertex_count() const {
    return vertex_count;
}

bool Mesh::does_have_uvs() const {
    return has_uvs;
}

bool Mesh::does_have_normals() const {
    return has_normals;
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

struct TextureParseData {
    std::string texture_path;
    bool clamp = false;
    Vector3 offset{0.0f};
    Vector3 scale{1.0f};
    float bm = 1.0f;
};

float float_parse_with_default_value(char *c, char*&end, const float default_value) {
    char* start = c;
    float f = std::strtof(c, &end);
    if (start != end) {
        return f;
    }
    end--;
    return default_value;
}

TextureParseData parse_mtl_texture_statement(const std::string& statement) {
    auto *c = const_cast<char *>(statement.c_str());
    TextureParseData data{};
    while (c != nullptr) {
        if (*c == ' ' or *c == '\t') {
            c++;
            continue;
        }

        if (*c == '-') {
            const char c_next = *(c+1);
            if (c_next == 'o' and *(c+2) == ' ') { // -o 1.00 1.00 1.00
                data.offset.x = std::strtof(c + 2, &c);
                data.offset.y = float_parse_with_default_value(c+1, c, 0.0f);
                data.offset.z = float_parse_with_default_value(c+1, c, 0.0f);
                continue;
            } else if (c_next == 's' and *(c+2) == ' ') { // -s 1.00 1.00 1.00
                data.scale.x = std::strtof(c + 2, &c);
                data.scale.y = float_parse_with_default_value(c+1, c, 1.0f);
                data.scale.z = float_parse_with_default_value(c+1, c, 1.0f);
                continue;
            } else if (c_next == 'b' and *(c+2) == 'm' and *(c+3)==' ') { // -bm 1.00
                data.bm =  std::strtof(c + 3, &c);
                continue;
            } else if (c_next == 'c' and *(c+2) == 'l' and *(c+3)=='a' and *(c+4)=='m' and *(c+5)=='p' and *(c+6) == ' ') {
                data.clamp = *(c + 8) == 'n' ? true : false; // -clamp on | off
                c += 10;
                continue;
            }
            // if the opption is not supported, just continue until a new option is found OR we found the end => go to the last space
            Engine::debug_warning("Unsupported MTL option found: " + statement + " IGNORING IT");
            c++;
            if (c == nullptr)
                break;
            char* last_space = nullptr;
            while (*c != '-') {
                if (*c == ' ' or *c == '\t') {
                    last_space = c;
                }
                c++;

                if (c == nullptr) {
                    if (last_space == nullptr)
                        break;
                    c = last_space;
                }
            }
            continue;
        }
        data.texture_path = statement.substr(c-statement.c_str());
        break;
    }
    return data;
}



/// Parses a mtl file
/// @param file_path .mtl file path
/// @param has_normals if .obj file that links this mtl has normal data (so appropriate shaders can be chosen)
/// @param has_uvs -- // -- has uvs data (so appropriate shaders can be chosen)
/// @param tangent_maps_action what to do with Normal,Bump maps or any other maps that use tangent calculations - Auto - If material need them, have them OR Force ignore the maps OR Force shaders to support tangents, just in case
/// @returns an unordered map [material name : std::shared_ptr of instanced material]
std::unordered_map<std::string, std::shared_ptr<Material>> parse_mtl_file(const char* file_path, bool has_normals, bool has_uvs, const Model::TangentAction& tangent_maps_action) {
    std::unordered_map<std::string, std::shared_ptr<Material>> materials;

    std::ifstream file(file_path);
    if (!file.good()) {
        std::cerr << "ENGINE ERROR: FAILED LOADING .mtl file: " << file_path << std::endl;
        return materials;
    }
    std::string line;

    // parsed values
    std::shared_ptr<Material> mat = nullptr;

    auto template_shader_program = tangent_maps_action != Model::FORCE_GENERATE_ALL ?
        ge.shaders.get_base_material(has_uvs, has_normals)->get_shader_program()
            :
        ge.shaders.get_base_material(Shaders::VERTEX_UV_NORMAL_TANGENT)->get_shader_program();

    while (std::getline(file, line)) {
        if (line[0] == 'n') {
            auto material_name = after_char(line, ' ');
            mat = std::make_shared<Material>(template_shader_program);
            mat->set_uniform("material.has_albedo_texture", false);
            materials[material_name] = mat;
        } else if (line[0] == '\0') {
            mat = nullptr;
        }

        //
        if (mat != nullptr) {
            unsigned int char_offset = 0;

            while (std::isspace(line[char_offset])) {
                char_offset += 1;
            }

            const char * l = line.c_str() + 2 + char_offset;
            if (line[char_offset] == 'K') {
                char * end;
                auto v3 = Vector3{};
                v3.x = std::strtof(l, &end);
                v3.y = std::strtof(end, &end);
                v3.z = std::strtof(end, nullptr);

                if (line[char_offset + 1] == 'a' and has_normals)
                    mat->set_uniform("material.ambient", v3);
                else if (line[char_offset + 1] == 'd')
                    mat->set_uniform("material.diffuse", v3);
                else if (line[char_offset + 1] == 's' and has_normals)
                    mat->set_uniform("material.specular", v3);
            } else if (line[char_offset] == 'N' and has_normals) {
                if (line[char_offset + 1] == 'i')
                    mat->set_uniform("material.shininess", std::strtof(l, nullptr));
            } else if (line[char_offset] == 'm') {
                auto data = parse_mtl_texture_statement(after_char(line, ' '));
                data.texture_path = (std::filesystem::path(file_path).parent_path() / data.texture_path).string();

                if (line[char_offset + 5] == 'd') {
                    mat->set_uniform("material.has_albedo_texture", true);
                    // assume sRGB for diffuse textures
                    auto texture = std::make_shared<Texture>(data.texture_path.c_str(), ge.get_gamma_correction());
                    mat->set_uniform("material.albedo_texture", texture);
                } else if (tangent_maps_action != Model::FORCE_NO_GENERATION and line[char_offset + 4] == 'B') {
                    // update shader to support normal textures
                    if (mat->get_shader_program_id() != ge.shaders.get_base_material(Shaders::VERTEX_UV_NORMAL_TANGENT)->get_shader_program_id()) {
                        mat->shader_program_switch(ge.shaders.get_base_material(Shaders::VERTEX_UV_NORMAL_TANGENT)->get_shader_program());
                    }

                    mat->set_uniform("material.has_normal_texture", true);
                    auto texture = std::make_shared<Texture>(data.texture_path.c_str());
                    mat->set_uniform("material.normal_texture", texture);
                }
            }
        }
    }

    return materials;
}


// PARSES OBJ FILE AS A MODEL (separating vertex groups, parsing materials from mtllib)
void parse_obj_file(const char* file_path, std::vector<float> (&vertex_data_vec)[4], std::vector<std::vector<size_t>>& vertex_groups, std::vector<std::shared_ptr<Material>>& materials, bool &has_uvs, bool &has_normals, const Model::TangentAction &tangent_action) {
    std::ifstream file(file_path);
    if (!file.good()) {
        std::cerr << "ENGINE ERROR: FAILED LOADING .obj file: " << file_path << std::endl;
        return;
    }
    std::string line;

    size_t current_group_idx = -1;

    has_normals = false;
    has_uvs = false;

    std::filesystem::path mtl_path;
    bool generated_materials = false;
    std::unordered_map<std::string, std::shared_ptr<Material>> mtl_materials;

    // PARSING OF .OBJ FILE
    // go line by line
    // TODO seems a bit slow, after parsing unordered map is instant, this will be the constraint
    while (std::getline(file, line)) {
        // mtllib X
        if (line[0] == 'm') {
            auto mlt_file = normalize_path(after_char(line, ' '));
            std::filesystem::path path(file_path);
            mtl_path = path.parent_path() / mlt_file;
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
            if (data_type != 1) {
                vertex_data_vec[data_type].push_back(std::strtof(end, nullptr));
            }
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
            // generate materials only once we know if .obj has normals and uvs
            if (!generated_materials) {
                mtl_materials = parse_mtl_file(mtl_path.string().c_str(), has_normals, has_uvs, tangent_action);
                generated_materials = true;
            }


            std::string material_name = after_char(line, ' ');
            materials.push_back(mtl_materials[material_name]);

            // create
            vertex_groups.emplace_back();
            current_group_idx += 1;
        }
    }
}


// PARSE OBJ FILE AS A UNIFORM MESH WITH NO MATERIAL
void parse_obj_file(const char* file_path, std::vector<float> (&vertex_data_vec)[4], std::vector<size_t>& vertex_group, bool &has_uvs, bool &has_normals) {
    std::ifstream file(file_path);
    if (!file.good()) {
        std::cerr << "ENGINE ERROR: FAILED LOADING .obj file: " << file_path << std::endl;
        return;
    }
    std::string line;
    has_normals = false;
    has_uvs = false;

    // PARSING OF .OBJ FILE
    // go line by line
    // TODO seems a bit slow, after parsing, unordered map is instant, this will be the constraint
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
            if (data_type != 1) {
                vertex_data_vec[data_type].push_back(std::strtof(end, nullptr));
            }
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


void construct_mesh_data_from_parsed_obj_data(const std::vector<float> (&vertex_data_vec)[4], const std::vector<size_t>& vertex_triplets, const bool has_normals, const bool has_texture_cords, std::vector<float>& out_vertex_data, std::vector<unsigned int>& out_indices) {
    std::unordered_map<UniqueVertexDataPoint, unsigned int, UniqueVertexDataPointHash> unique_vertex_data_points;

    unsigned int vertex_data_group_size = 1 + has_normals + has_texture_cords;

    unsigned int indecy_count = 0;

    const bool needs_tangents = !vertex_data_vec[3].empty();

    size_t face_counter = 0;
    char vertex_within_face_counter = 0;

    for (size_t i = 0; i < (vertex_triplets.size() / vertex_data_group_size); i++) {
        if (needs_tangents) {
            // face counter, by incrementing every 3 vertexes
            if (vertex_within_face_counter >= 3) {
                vertex_within_face_counter = 0;
                face_counter += 1;
            }
            vertex_within_face_counter += 1;
        }

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
            // add tangents if needed
            if (needs_tangents) {
                out_vertex_data.push_back(vertex_data_vec[3][face_counter * 3]);
                out_vertex_data.push_back(vertex_data_vec[3][face_counter * 3 + 1]);
                out_vertex_data.push_back(vertex_data_vec[3][face_counter * 3 + 2]);
            }
        } else {
            out_indices.push_back(match->second);
        }
    }
}


void calculate_tangents(std::vector<float> (&vertex_data_vec)[4], const std::vector<size_t>& vertex_group) {
    // loop over faces
    for (size_t i = 0; i < vertex_group.size() / 9; i++) {
        // positions
        glm::vec3 point_a(vertex_data_vec[0][(vertex_group[i * 9] - 1) * 3],  vertex_data_vec[0][(vertex_group[i * 9] - 1) * 3 + 1], vertex_data_vec[0][(vertex_group[i * 9] - 1) * 3 + 2]);
        glm::vec3 point_b(vertex_data_vec[0][(vertex_group[i * 9 + 3] - 1) * 3],  vertex_data_vec[0][(vertex_group[i * 9 + 3] - 1) * 3 + 1], vertex_data_vec[0][(vertex_group[i * 9 + 3] - 1) * 3 + 2]);
        glm::vec3 point_c(vertex_data_vec[0][(vertex_group[i * 9 + 6] - 1) * 3],  vertex_data_vec[0][(vertex_group[i * 9 + 6] - 1) * 3 + 1], vertex_data_vec[0][(vertex_group[i * 9 + 6] - 1) * 3 + 2]);
        // texture coordinates
        glm::vec2 uv1(vertex_data_vec[1][(vertex_group[i * 9 + 1] - 1) * 2], vertex_data_vec[1][(vertex_group[i * 9 + 1] - 1) * 2 + 1]);
        glm::vec2 uv2(vertex_data_vec[1][(vertex_group[i * 9 + 4] - 1) * 2], vertex_data_vec[1][(vertex_group[i * 9 + 4] - 1) * 2 + 1]);
        glm::vec2 uv3(vertex_data_vec[1][(vertex_group[i * 9 + 7] - 1) * 2], vertex_data_vec[1][(vertex_group[i * 9 + 7] - 1) * 2 + 1]);

        glm::vec3 edge1 = point_b - point_a;
        glm::vec3 edge2 = point_c - point_a;
        glm::vec2 deltaUV1 = uv2 - uv1;
        glm::vec2 deltaUV2 = uv3 - uv1;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        // save 1 tangent vector per face
        vertex_data_vec[3].push_back(f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x));
        vertex_data_vec[3].push_back( f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y));
        vertex_data_vec[3].push_back(f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z));
    }
}


Mesh::Mesh(const char* file_path, bool generate_tangents) {
    vertex_buffer_object = -1;
    vertex_array_object = -1;
    element_buffer_object = -1;

    // define structures
    std::vector<float> vertex_data_vec[4];
    std::vector<size_t> vertex_group;

    // use structures to parse .obj file
    parse_obj_file(file_path, vertex_data_vec, vertex_group, has_uvs, has_normals);

    // calculate tangents if told so
    if (generate_tangents)
        calculate_tangents(vertex_data_vec, vertex_group);
    has_tangents = generate_tangents;

    // define new structures, for reordering
    std::vector<float> vertex_data;
    std::vector<unsigned int> indices;

    // use structures to create correctly formated values for Mesh

    // vertex_groups[0], because usually we would loop through all the groups and create the appropriate Mesh
    // but this is just a mesh, so we merged_all_groups, and now we work with only one group
    construct_mesh_data_from_parsed_obj_data(vertex_data_vec, vertex_group, has_normals, has_uvs, vertex_data, indices);
    load_mesh_to_gpu(&vertex_data, &indices, has_uvs, has_normals, generate_tangents);
}

Model::Model(const char* file_path, const TangentAction& action) {
    // define structures
    std::vector<float> vertex_data_vec[4];
    std::vector<std::vector<size_t>> vertex_groups;

    // use structures to parse .obj file
    parse_obj_file(file_path, vertex_data_vec, vertex_groups, materials, has_uvs, has_normals, action);

    for (size_t i = 0; i < vertex_groups.size(); ++i) {
        auto& vertex_group = vertex_groups[i];
        if (vertex_group.empty())
            continue;

        // define new structures, for reordering
        std::vector<float> vertex_data;
        std::vector<unsigned int> indices;

        // tangents
        bool will_have_tangents = action == FORCE_GENERATE_ALL;
        if (action == AUTO_GENERATE) {
            // generate if material linked to this mesh supports tangents i.e. the mtl paser found a map texture requiring tangents
            will_have_tangents = materials.size() >= i + 1 and (materials[i]->get_shader_program_id() == ge.shaders.get_base_material(Shaders::VERTEX_UV_NORMAL_TANGENT)->get_shader_program_id());
        }
        if (will_have_tangents)
            calculate_tangents(vertex_data_vec, vertex_group);

        // use structures to create correctly formated values for Mesh
        construct_mesh_data_from_parsed_obj_data(vertex_data_vec, vertex_group, has_normals, has_uvs, vertex_data, indices);

        auto msh = std::make_shared<Mesh>(&vertex_data, &indices, has_uvs, has_normals, will_have_tangents);
        meshes.push_back(msh);
    }
}

std::shared_ptr<Material> Model::get_material(size_t index) const {
    return materials[index];
};

std::shared_ptr<Mesh> Model::get_mesh(size_t index) const {
    return meshes[index];
}

size_t Model::get_mesh_count() const {
    return meshes.size();
}

bool Model::get_has_uvs() const {
    return has_uvs;
}

bool Model::get_has_normals() const {
    return has_normals;
}

Mesh::~Mesh() {
    glDeleteVertexArrays(1,  &vertex_array_object);
    glDeleteBuffers(1, &vertex_buffer_object);
    glDeleteBuffers(1, &element_buffer_object);
}

void Meshes::load_base_meshes()
{
    std::cout << "ENGINE MESSAGE: Default meshes created" << std::endl;
    plane = std::make_shared<Mesh>(&PLANE_VERTEX_DATA, &PLANE_INDICES, true, true);
    cube = std::make_shared<Mesh>(&CUBE_VERTEX_DATA, &CUBE_INDICES, true, true);
}

void Meshes::unload_base_meshes()
{
    plane = nullptr;
    cube = nullptr;
}


