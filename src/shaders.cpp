#include "shaders.hpp"
#include <iostream>
#include <fstream>
#include <utility>
#include "graphicengine.hpp"
#include "gtc/type_ptr.hpp"


Shader::Shader(const char *file_path, const ShaderType shader_type, const std::string &define_header) {
    id = glCreateShader(shader_type);

    std::string shader_string;

    std::ifstream file(file_path);

    if (!file.good()) {
        glDeleteShader(id);
        Engine::debug_error("Failed to load shader file: " + std::string(file_path));
        return;
    }

    std::string line;
    bool prepended_header = false;
    while (std::getline(file, line)) {
        shader_string += line + '\n';
        if (!prepended_header and line[0] == '#' and line[1] == 'v') {
            prepended_header = true;
            shader_string += define_header;

            // add obligatory header (if not needed preprocessor will strip it away anyway)
            if (shader_type == GL_FRAGMENT_SHADER) {
                shader_string += "#define NR_POINT_LIGHTS " + std::to_string(ge.lights.MAX_NR_POINT_LIGHTS) + "\n";
                shader_string += "#define NR_DIRECTIONAL_LIGHTS " + std::to_string(ge.lights.MAX_NR_DIRECTIONAL_LIGHTS) + "\n";
            }
        }
    }
    file.close();

    file_path = shader_string.c_str();
    glShaderSource(id, 1, &file_path, nullptr);
    glCompileShader(id);

    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(id, 512, nullptr, infoLog);
        Engine::debug_error("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" + std::string(infoLog));
    } else {
        Engine::debug_message("Shader successfully compiled!");
    }
}


Shader::Shader(const std::string &shader_code, const ShaderType shader_type) {
    id = glCreateShader(shader_type);
    const char *content = shader_code.c_str();
    glShaderSource(id, 1, &content, nullptr);
    glCompileShader(id);


    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(id, 512, nullptr, infoLog);
        Engine::debug_error("ERROR::SHADER::COMPILATION_FAILED\n" + std::string(infoLog));
    } else {
        Engine::debug_message("Shader successfully compiled from std::string");
    }
}

unsigned int Shader::get_id() const {
    return id;
}


Shader::~Shader() {
    glDeleteShader(id);
}


ShaderProgram::ShaderProgram(unsigned int _id) {
    id = _id;
    ge.shaders.add_shader_id_use(id);
}

ShaderProgram::ShaderProgram(const ShaderProgram &sp) {
    id = sp.id;
    ge.shaders.add_shader_id_use(id);
}

ShaderProgram::ShaderProgram(ShaderProgram &&other) noexcept {
    id = other.id;
    other.id = -1;
}

ShaderProgram& ShaderProgram::operator=(const ShaderProgram &other) {
    ge.shaders.remove_shader_id_use(id);
    id = other.id;
    ge.shaders.add_shader_id_use(id);
    return *this;
}


ShaderProgram& ShaderProgram::operator=(ShaderProgram &&other) noexcept {
    if (this == &other)
        return *this;

    ge.shaders.remove_shader_id_use(id);
    id = other.id;
    other.id = -1;

    return *this;
}

ShaderProgram::~ShaderProgram() { // DESTRUCTOR WARNING !!
    ge.shaders.remove_shader_id_use(id);
}


ShaderProgram::ShaderProgram(const Shader &vertex_shader, const Shader &fragment_shader) {
    id = glCreateProgram();
    glAttachShader(id, vertex_shader.get_id());
    glAttachShader(id, fragment_shader.get_id());
    glLinkProgram(id);

    int success;
    char infoLog[512];
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(id, 512, nullptr, infoLog);
        Engine::debug_error("Program link error:\n" + std::string(infoLog));
    }

    glUniformBlockBinding(id, glGetUniformBlockIndex(id, "MATRICES"), 0);

    const auto light_block_idx = glGetUniformBlockIndex(id, "LIGHTS");
    if (light_block_idx != GL_INVALID_INDEX) {
        glUniformBlockBinding(id, light_block_idx, 1);
    }

    ge.shaders.add_shader_id_use(id);
}

unsigned int ShaderProgram::get_id() const {
    return id;
}

void ShaderProgram::use() const {
    glUseProgram(id);
}

void ShaderProgram::set_uniform(const char* uniform_name, glm::mat4 matrix) const {
    glUniformMatrix4fv(glGetUniformLocation(id, uniform_name), 1, GL_FALSE, &matrix[0][0]);
}

void ShaderProgram::set_uniform(const char* uniform_name, float val) const {
    glUniform1f(glGetUniformLocation(id, uniform_name), val);
}

void ShaderProgram::set_uniform(const char* uniform_name, glm::vec3 val) const {
    glUniform3fv(glGetUniformLocation(id, uniform_name), 1, &val[0]);
}

unsigned int ShaderProgram::get_uniform_location(const char *uniform_name) const {
    return glGetUniformLocation(id, uniform_name);
}

Material::Material(const ShaderProgram &_shader_program) : shader_program(_shader_program) {
    id = ge.shaders.get_material_identificator();
}

void Material::apply_uniform_values() const {
    uniform_map::const_iterator it;

    // (only used when bindless textures are NOT supported)
    int bind_texture_slot = 0;

    for (it = uniforms.begin(); it != uniforms.end(); it++) {
        int uniform_loc = it->first;
        uniform_variant value = it->second;

        if (std::holds_alternative<float>(value)) {
            glUniform1f(uniform_loc, std::get<float>(value));
        }
        else if (std::holds_alternative<Vector3>(value)) {
            const auto vec = std::get<Vector3>(value);
            glUniform3f(uniform_loc, vec.x, vec.y, vec.z);
        }
        else if (std::holds_alternative<std::shared_ptr<Texture>>(value)) {
            if (ge.are_bindless_textures_supported()) {
                glProgramUniformHandleui64ARB(shader_program.get_id(), uniform_loc, std::get<std::shared_ptr<Texture>>(value)->handle);
            } else {
                glActiveTexture(GL_TEXTURE0 + bind_texture_slot);
                glBindTexture(GL_TEXTURE_2D, std::get<std::shared_ptr<Texture>>(value)->id);
                glUniform1i(uniform_loc, bind_texture_slot);
                bind_texture_slot += 1;
            }
        } else if (std::holds_alternative<bool>(value)) {
            glUniform1i(uniform_loc, std::get<bool>(value));
        } else if (std::holds_alternative<int>(value)) {
            glUniform1i(uniform_loc, std::get<int>(value));
        } else if (std::holds_alternative<Color>(value)) {
            const auto color = std::get<Color>(value);
            glUniform4f(uniform_loc, color.r, color.g, color.b, color.a);
        }
    }
}

uint64_t Material::get_id() const {
    return id;
}

const ShaderProgram& Material::get_shader_program() const {
    return shader_program;
}

unsigned int Material::get_shader_program_id() const {
    return shader_program.get_id();
}

std::shared_ptr<Material> Material::copy() const {
    auto mat = std::make_shared<Material>(shader_program);
    mat->uniforms = uniforms;
    return mat;
}

void Material::rebind_uniforms() {
    for (auto it = uniform_name_to_loc.begin(); it != uniform_name_to_loc.end(); it++) {
        int new_loc = get_uniform_location(it->first.c_str());
        // edit key in uniforms map
        auto nh = uniforms.extract(it->second);
        nh.key() = new_loc;
        uniforms.insert(std::move(nh));
        //
        uniform_name_to_loc[it->first] = new_loc;
    }
}

void Material::shader_program_switch(ShaderProgram new_sp) {
    // check if Material not in use
    auto self = shared_from_this();
    if (ge.thing_ids_by_shader_program.find(self) != ge.thing_ids_by_shader_program.end()) {
        Engine::debug_error("Material (id " + std::to_string(id) + ") is already in use ShaderProgram can't be changed no longer. Create a new Material and set it's uniforms to the uniforms here and call rebind_uniforms()");
        return;
    }
    shader_program = std::move(new_sp);
    rebind_uniforms();

    /* THEORETICAL IN ENGINE RESORT, BUT STILL DOESN'T UPDATE ENTITIES THUS (SCRAPED FOR NOW)
     *if (resort_engine_map) {
        // find all
        auto self = shared_from_this();
        const auto [fst, snd] = ge.thing_ids_by_shader_program.equal_range(self);

        // get iterators
        std::vector<decltype(ge.thing_ids_by_shader_program)::node_type> nhs;
        for (auto it = fst; it != snd;) {
            nhs.push_back(ge.thing_ids_by_shader_program.extract(it++));
        }

        // extract and re-insert
        for (auto &nh : nhs) {
            ge.thing_ids_by_shader_program.insert(std::move(nh));
        }
    }*/
}


int Material::get_uniform_location(const char *uniform_name) const {
    shader_program.use();
    return glGetUniformLocation(shader_program.get_id(), uniform_name);
}

void Material::set_uniform(const char *uniform_name, const uniform_variant &val) {
    int loc = get_uniform_location(uniform_name);
    uniform_name_to_loc[uniform_name] = loc;
    uniforms[loc] = val;
}

uniform_variant Material::get_uniform(const char *uniform_name) const {
    auto it = uniforms.find(get_uniform_location(uniform_name));
    if (it != uniforms.end())
        return it->second;
    Engine::debug_error("Get failed " + std::string(uniform_name) + " not found");
    return false;
}

void Shaders::add_shader_id_use(const unsigned int sp_id) {
    shader_programs_id_used[sp_id] += 1;
}

void Shaders::remove_shader_id_use(const unsigned int sp_id) {
    if (sp_id == -1) {
        return;
    }
    if (!shader_programs_id_used.contains(sp_id)) {
        Engine::debug_error("sp_id: " + std::to_string(sp_id) + " NOT FOUND !! ");
        return;
    }
    shader_programs_id_used[sp_id] -= 1;

    if (shader_programs_id_used[sp_id] == 0) {
        glDeleteProgram(sp_id);
        shader_programs_id_used.erase(sp_id);
        Engine::debug_message("deleting shader program " + std::to_string(sp_id));
    }
}

unsigned int Shaders::get_shader_use_by_id(unsigned int sp_id) {
    auto it = shader_programs_id_used.find(sp_id);
    if (it != shader_programs_id_used.end())
        return it->second;
    return -1;
}


void Shaders::debug_show_shader_program_use() {
    std::cout << "ENGINE MESSAGE: SHADER PROGRAM USE DEBUG" << std::endl;
    std::cout << "! - ID X USE COUNT -  !" << std::endl;
    for (auto it = shader_programs_id_used.begin(); it != shader_programs_id_used.end(); it++) {
        std::cout << it->first << " X " << it->second << std::endl;
    }
}


void Shaders::setup_placeholder_textures() {
    texture_placeholders[WHITE] = std::make_shared<Texture>(Color::WHITE);
    texture_placeholders[NORMAL_MAP] = std::make_shared<Texture>(Color{0.5, 0.5, 1.0});
}

std::shared_ptr<Texture> Shaders::get_placeholder_texture(const PlaceholderTextures identifier) const {
    return texture_placeholders[identifier];
}


void Shaders::setup_base_materials() {
    base_materials[0] = std::make_shared<Material>(phong_shader_program_gen(true, false));
    base_materials[0]->set_uniform("material.ambient", Vector3(0.2f));
    base_materials[0]->set_uniform("material.diffuse", Color::WHITE.no_alpha());
    base_materials[0]->set_uniform("material.specular", Color::WHITE.no_alpha());
    base_materials[0]->set_uniform("material.shininess", 32.0f);
    base_materials[0]->set_uniform("material.albedo_texture", get_placeholder_texture(WHITE));

    base_materials[1] = std::make_shared<Material>(phong_shader_program_gen(true, true));
    base_materials[1]->set_uniform("material.ambient", Vector3(0.2f));
    base_materials[1]->set_uniform("material.diffuse", Color::WHITE.no_alpha());
    base_materials[1]->set_uniform("material.specular", Color::WHITE.no_alpha());
    base_materials[1]->set_uniform("material.shininess", 32.0f);
    base_materials[0]->set_uniform("material.albedo_texture", get_placeholder_texture(WHITE));

    base_materials[2] = std::make_shared<Material>(phong_shader_program_gen(false, false));
    base_materials[2]->set_uniform("material.ambient", Vector3(0.2f));
    base_materials[2]->set_uniform("material.diffuse", Color::WHITE.no_alpha());
    base_materials[2]->set_uniform("material.specular", Color::WHITE.no_alpha());
    base_materials[2]->set_uniform("material.shininess", 32.0f);

    base_materials[3] = std::make_shared<Material>(no_normal_program_gen(true));
    base_materials[3]->set_uniform("material.diffuse", Color::WHITE.no_alpha());
    base_materials[3]->set_uniform("material.albedo_texture", get_placeholder_texture(WHITE));

    base_materials[4] = std::make_shared<Material>(no_normal_program_gen(false));
    base_materials[4]->set_uniform("material.diffuse", Color::WHITE.no_alpha());


}

std::shared_ptr<Material> Shaders::get_base_material(const bool with_uvs, const bool with_normals, const bool with_tangents) {
    if (with_uvs and with_normals) {
        if (!with_tangents) {
            return base_materials[VERTEX_UV_NORMAL];
        } else {
            return base_materials[VERTEX_UV_NORMAL_TANGENT];
        }
    }
    if (with_tangents) {
        Engine::debug_error("Material with NO UVs or NO NORMALs makes questionable sense. NOT SUPPORTED. Returning nullptr");
        return nullptr;
    }
    // relying
    if (with_normals) {
        return base_materials[VERTEX_NORMAL];
    }
    if (with_uvs) {
        return base_materials[VERTEX_UV];
    }
    return base_materials[VERTEX];
}

std::shared_ptr<Material> Shaders::get_base_material(const BaseMaterial identifier) {
    return base_materials[identifier];
}

void Shaders::clear_base_material(const size_t index) {
    base_materials[index] = nullptr;
}

uint64_t Shaders::get_material_identificator() {
    next_material_id += 1;
    return next_material_id - 1;
}



// SHADER GEN
Shader Shaders::base_vertex_shader_gen(const bool support_uv, const bool support_normal, const bool support_tangents) {
    std::string define_header;
    define_header += support_uv ? "#define HAS_UV\n" : "";
    define_header += support_normal ? "#define HAS_NORMALS\n" : "";

    // tangent logic
    if (support_tangents) {
        define_header += "#define HAS_TANGENTS\n";
        if (!support_uv or !support_normal) {
            Engine::debug_error("Tangent shader with NO UVs or NO NORMALs makes questionable sense. NOT SUPPORTED. DON'T DO IT");
        }
    }
    // generate shader
    return Shader{"engine/res/shaders/vertex_shader_template.glsl", Shader::VERTEX_SHADER, define_header};
}


Shader Shaders::base_phong_shader_gen(const bool support_uv, const bool support_tangents) const {
    std::string define_header;
    define_header += support_uv ? "#define HAS_UV\n" : "";
    define_header += bindless_textures_supported ? "#define USE_BINDLESS\n" : "";

    // tangent logic
    if (support_tangents) {
        define_header += "#define HAS_TANGENTS\n";
        if (!support_uv) {
            Engine::debug_error("Tangent shader with is illegal. NOT SUPPORTED. DON'T DO IT");
        }
    }
    // generate shader
    return Shader{"engine/res/shaders/obj_phong.glsl", Shader::FRAGMENT_SHADER, define_header};
}


Shader Shaders::base_no_normal_shader_gen(bool support_uv) const {
    std::string define_header;
    define_header += support_uv ? "#define HAS_UV\n" : "";
    define_header += bindless_textures_supported ? "#define USE_BINDLESS\n" : "";
    return Shader{"engine/res/shaders/obj_no_normal.glsl", Shader::FRAGMENT_SHADER, define_header};
}


ShaderProgram Shaders::phong_shader_program_gen(bool has_uvs, bool has_tangents) const {
    ShaderProgram sp {base_vertex_shader_gen(has_uvs, true, has_tangents), base_phong_shader_gen(has_uvs, has_tangents)};
    return sp;
}


ShaderProgram Shaders::no_normal_program_gen(bool has_uvs) const {
    ShaderProgram sp {base_vertex_shader_gen(has_uvs, false), base_no_normal_shader_gen(has_uvs)};
    return sp;
}
