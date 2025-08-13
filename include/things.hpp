#ifndef THINGS_H
#define THINGS_H
#pragma once
#include <string>
#include <glm/glm.hpp>
#include "coordinates.h"
#include "shaders.hpp"


class Camera {
public:
    glm::mat4 view{};
    glm::mat4 projection{};
    Transform transform{};
    float near_plane = 0;
    float far_plane = 0;
    float fov = 0;

    explicit Camera(const glm::mat4 &projection_matrix);
    explicit Camera(float _fov, float _far_plane, float _near_plane);
    explicit Camera(float _far_plane, float _near_plane);
    void change_resolution(int width, int height);
    void transform_to_view_matrix();
};


class Thing {
public:
    bool renderable;
    bool updatable;

    Thing (bool is_renderable, bool is_updatable);

    virtual void render(Camera *);
    virtual void update();

    virtual ~Thing();
};

class SpatialThing : public Thing {
public:
    Transform transform;
    SpatialThing(bool is_renderable, bool is_updatable);
};


class GeometryThing : public SpatialThing {
public:
    unsigned int vs_uniform_transform_loc;
    unsigned int vs_uniform_view_loc;
    unsigned int vs_uniform_projection_loc;
    Mesh* mesh;
    ShaderProgram* shader_program;

    GeometryThing (Mesh* _mesh, ShaderProgram* _shader_program);

    void update() override;

    void render(Camera * from_camera) override;
};

#endif //THINGS_H
