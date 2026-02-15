#ifndef SHADERS_HPP
#define SHADERS_HPP
#pragma once
#include <span>
#include <vector>
#include <glm/glm.hpp>
#include <string>
#include <variant>
#include <memory>
#include <map>
#include <array>
#include "textures.hpp"

typedef std::variant<float, glm::vec3, std::shared_ptr<Texture>, bool>  uniform_variant;
typedef std::map<int, uniform_variant> uniform_map;

/// Shader creator tool. Loads and compiles a shader, either from a file or from code directly. After compilation holds OpenGL id. Is used to create a ShaderProgram.
/// @ingroup Resource
/// @note This resource is not ment to be used as a std::shared_ptr and has no central counter. You just create it, use it, and once you don't need it delete it.
/// @warning If you try to copy this class somewhere, the deconstructor will be called and the shader will no longer exist on the GPU thus the copy will be useless, even harmful.
class Shader {
    /// OpenGL Shader ID
    unsigned int id = -1;
public:
    enum ShaderType {
        VERTEX_SHADER = GL_VERTEX_SHADER,
        FRAGMENT_SHADER = GL_FRAGMENT_SHADER,
    };
    /// read-only id getter
    [[nodiscard]] unsigned int get_id() const;

    /// Shader Constructor and Compiler from file_path
    /// @param file_path file path of shader code
    /// @param shader_type Whether shader code is a VERTEX shader or a FRAGMENT shader
    /// @param define_header a block of code that gets pasted in front, usually for #define X, which for example define NR_POINT_LIGHTS (more at Lights page)
    Shader(const char* file_path, ShaderType shader_type, const std::string &define_header = "");

    /// Shader Constructor and Compiler from code inside std::string
    /// @param shader_code the shader code in a string
    /// @param shader_type Whether shader code is a VERTEX shader or a FRAGMENT shader
    Shader(const std::string &shader_code, ShaderType shader_type);

    /// Shader Deconstructor. Deletes shader from GPU
    /// @note After a Shader is linked to a ShaderProgram, it is normal to delete the shader as it's already "inside" the ShaderProgram.
    ~Shader();
};


/// ShaderProgram is created by linking a VERTEX and a FRAGMENT SHADER.
class ShaderProgram {
    /// OpenGL Shader Program ID
    unsigned int id = -1;
public:
    /// Getter for read-only id
    unsigned int get_id() const;

    /// ShaderProgram creator based on linking a VERTEX and a FRAGMENT shader
    /// @param vertex_shader a vertex shader
    /// @param fragment_shader a fragment shader
    ShaderProgram(const Shader &vertex_shader, const Shader &fragment_shader);
    /// ShaderProgram creator based on an already existing ShaderProgram ID (basically for copying)
    explicit ShaderProgram(unsigned int _id);
    /// ShaderProgram creator based on an already existing ShaderProgram (basically for copying)
    ShaderProgram(const ShaderProgram& sp);

    /// ShaderProgram Deconstrutor
    /// @warning touches the ge global variable, thus it has to exist during deconstruction, in all normal use cases this is ok, but just if you intend on getting intend on doing anything crazy consider yourself to be warned.
    ~ShaderProgram();

    /// Says to OpenGL that this ShaderProgram should be used on the following geometry
    /// @note not really intended for a user use-case
    void use() const;

    /// One time uniform setter
    /// @param uniform_name the name of the uniform in shader code
    /// @param matrix a Matrix from the glm library
    /// @note use() has to be called for the setting to take effect
    void set_uniform(const char* uniform_name, glm::mat4 matrix) const;

    /// One time uniform setter
    /// @param uniform_name the name of the uniform in shader code
    /// @param val a vec3 from the glm library
    /// @note use() has to be called for the setting to take effect
    void set_uniform(const char* uniform_name, glm::vec3 val) const;

    /// One time uniform setter
    /// @param uniform_name the name of the uniform in shader code
    /// @param val a float
    /// @note use() has to be called for the setting to take effect
    void set_uniform(const char* uniform_name, float val) const;

    ShaderProgram& operator=(const ShaderProgram& sp);

    /// return the id of the uniform based on name
    /// @param uniform_name the uniform name
    /// @note use() has to be called to get the correct answer
    unsigned int get_uniform_location(const char *uniform_name) const;
};


/// Holds uniform values of a ShaderProgram and is responsible for their correct usage.
/// It's the intended way of adding colors to Meshes.
class Material {
public:
    /// Unique material id
    uint64_t id = -1;
    /// ShaderProgram that is applied to the geometry
    ShaderProgram shader_program;
    /// Container for the shader values
    uniform_map shader_values = {};
    /// Construction from a shader program
    /// @param ShaderProgram
    explicit Material(const ShaderProgram &_shader_program);

    /// ONE TIME sets all uniform values saved by the material
    /// @note used by the renderer to material switch
    void set_uniform_values() const;

    /// Saves a uniform value and holds on this value util it's resaved. Primary way of changing material values.
    /// @param uniform_name name of the uniform you want to change
    /// @param val the value you want to change it to (limited by the uniform variant type)
    void save_uniform_value(const char* uniform_name, const uniform_variant &val);

    /// return the id of the uniform based on name
    /// @param uniform_name the uniform name
    unsigned int get_uniform_location(const char *uniform_name) const;
};

/// Helper Struct for a std::map in Engine class, that sorts renderable objects by shader_program first, material_id second
/// Useful for rendering order optimization.
struct MaterialSorter {
    bool operator()(const std::shared_ptr<Material>& m1, const std::shared_ptr<Material>& m2) const {
        if (m1->shader_program.get_id() != m2->shader_program.get_id()) {
            return m1->shader_program.get_id() < m2->shader_program.get_id();
        }
        return m1->id < m2->id;
    }
};


/// Shader Helper class. Holds base materials for .obj parser and convenience for the user.
class Shaders {
    /// central shader program use counter
    std::map<unsigned int, unsigned int> shader_programs_id_used = {};

    /// BASE SHADER PROGRAMS
    /// a set of shader programs useful for loading MTL materials,
    /// all here, so that they can be universally used across all loaded models
    /// @note all loaded in memory, unless clear_base_material(size_t index) is called
    /// LEGEND:
    /// 0 -> v
    /// 1 -> vn
    /// 2 -> vu
    /// 3 -> vnu
    /// PS: these sp can be useful if you don't want to use custom shaders
    std::array<std::shared_ptr<Material>, 4> base_materials{};
    /// AUTO increment Shader ID value
    uint64_t next_material_id = 0;
public:
    /// Base material INIT function (called in the Engine constructor)
    void setup_base_materials();
    /// Shader Program counter use add
    /// @note Ment for the engine
    void add_shader_id_use(unsigned int sp_id);
    /// Shader Program counter use remove
    /// @note Ment for the engine
    void remove_shader_id_use(unsigned int sp_id);
    /// Debug output Shader Program use table
    void debug_show_shader_program_use();
    /// Next ID material getter
    uint64_t get_material_identificator();

    /// Base material getter
    /// @param with_uvs whether you want the Material for a mesh with UVs
    /// @param with_normals whether you want the Material for a mesh with Normals
    /// @warning it's important that you choose the correct material for the Mesh, but most models are with UVs and NORMALS (usually you enter true, true)
    std::shared_ptr<Material> get_base_material(bool with_uvs, bool with_normals);

    /// Deletes any base material it truly not needed.
    /// @param index index of the material (0, 1, 2, 3 -> v, vn, vu, vnu)
    void clear_base_material(size_t index);

    // shader gen

    /// If this computer running the program supports bindless textures
    bool bindless_textures_supported = false;

    /// Generates a ShaderProgram with a basic phong lighting system
    /// @param has_uvs Whether you want the shader to be for a model with UVs (usually yes)
    [[nodiscard]] ShaderProgram phong_shader_program_gen(bool has_uvs) const;
    /// Generates a ShaderProgram with ambient lighting (ment for models with No normals)
    /// @param has_uvs Whether you want the shader to be for a model with UVs (usually yes)
    [[nodiscard]] ShaderProgram no_normal_program_gen(bool has_uvs) const;

    /// Generates a Vertex Shader applicable to 90% of situations.
    /// @param support_uv If it's ment for a model with UV coords
    /// @param support_normal If it's ment for a model with Normal coords
    static Shader base_vertex_shader_gen(bool support_uv = true, bool support_normal = true);

    /// Generates a basic phong lighting Fragment Shader
    /// @param support_uv Whether you want the shader to be for a model with UVs (usually yes)
    [[nodiscard]] Shader base_phong_shader_gen(bool support_uv = true) const;
    /// Generates a basic ambient lighting Fragment Shader
    /// @param support_uv Whether you want the shader to be for a model with UVs (usually yes)
    [[nodiscard]] Shader base_no_normal_shader_gen(bool support_uv = true) const;
};

#endif //SHADERS_HPP
