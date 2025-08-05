#ifndef THINGS_H
#define THINGS_H
#pragma once
#include <string>
#include <glm/glm.hpp>

#include "shaders.hpp"


class Camera {
public:
    glm::mat4 projection{};
    glm::mat4 view{};
    float near_plane = 0;
    float far_plane = 0;
    float fov = 0;

    explicit Camera(const glm::mat4 &projection_matrix);
    explicit Camera(float _fov, float _far_plane, float _near_plane);
    explicit Camera(float _far_plane, float _near_plane);
    void change_resolution(int width, int height);
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
    unsigned int vs_uniform_transform_loc;
    unsigned int vs_uniform_view_loc;
    unsigned int vs_uniform_projection_loc;
    Mesh* mesh;
    ShaderProgram* shader_program;

    GeometryThing (std::string object_name, Mesh* _mesh, ShaderProgram* _shader_program);

    void update() override;

    void render(Camera * from_camera) override;
};

#endif //THINGS_H
