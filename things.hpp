#ifndef THINGS_H
#define THINGS_H
#include <string>
#include <glm/glm.hpp>

class Mesh {
public:
    unsigned int vertex_buffer_object;
    unsigned int vertex_array_object;
    unsigned int element_buffer_object;
    size_t vertex_count = 0;

    Mesh(std::vector<float> vertices, std::vector<unsigned int> indices);
    Mesh();

    ~Mesh();
};

class Camera {
public:
    glm::mat4 projection{};
    glm::mat4 view{};

    explicit Camera(const glm::mat4 &projection_matrix);
};


class Thing {
public:
    std::string name;
    bool renderable;
    bool updatable;

    Thing (bool is_renderable, bool is_updatable, std::string object_name);

    virtual void render(Camera *);
    virtual void update();

    virtual ~Thing();
};

class SpatialThing : public Thing {
public:
    glm::mat4 transform{};
    SpatialThing(bool is_renderable, bool is_updatable, std::string object_name);
};


class GeometryThing : public SpatialThing {
public:
    unsigned int shader_program;
    unsigned int vs_uniform_transform_loc;
    unsigned int vs_uniform_view_loc;
    unsigned int vs_uniform_projection_loc;
    Mesh* mesh;

    GeometryThing (std::string object_name, Mesh* _mesh, const char* fragment_shader_path);

    ~GeometryThing() override;

    void update() override;

    void render(Camera * from_camera) override;
};

#endif //THINGS_H
