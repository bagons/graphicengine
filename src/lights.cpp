#include "lights.hpp"
#include <algorithm>
#include "graphicengine.hpp"
#include "glad/glad.h"
#include "things.hpp"
#include <iostream>


PointLight::PointLight(const glm::vec3 color, const float intensity, const bool is_updatable) : SpatialThing(false, is_updatable),
color(color), intensity(intensity) {

}

DirectionalLight::DirectionalLight(const glm::vec3 color, const float intensity, const glm::vec3 direction, const bool is_updatable) : Thing(false, is_updatable),
color(color), intensity(intensity), direction(direction) {

}

Lights::Lights(unsigned int MAX_NR_POINT_LIGHTS, unsigned int MAX_NR_DIRECTIONAL_LIGHTS, LightOverflowAction light_overflow_action) :
MAX_NR_POINT_LIGHTS(MAX_NR_POINT_LIGHTS), MAX_NR_DIRECTIONAL_LIGHTS(MAX_NR_DIRECTIONAL_LIGHTS), light_overflow_action(light_overflow_action) { }


Lights::~Lights() {
    glDeleteBuffers(1, &lights_ubo);
}

void Lights::init_central_light_system() {
    const unsigned int buffer_size = PointLight::STRUCT_BYTE_SIZE * MAX_NR_POINT_LIGHTS + DirectionalLight::STRUCT_BYTE_SIZE * MAX_NR_DIRECTIONAL_LIGHTS;

    // create a Uniform Buffer for light system
    glGenBuffers(1, &lights_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, lights_ubo);
    glBufferData(GL_UNIFORM_BUFFER, buffer_size, nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, 1, lights_ubo, 0, buffer_size);
}




void Lights::update(Position& camera_pos) {
    // SORT LIGHTS BY PROXIMITY IF ABOVE LIGHT LIMIT
    if (light_overflow_action == SORT_BY_PROXIMITY) {
        if (point_lights.size() > MAX_NR_POINT_LIGHTS) {
            std::ranges::nth_element
            (point_lights, point_lights.begin() + MAX_NR_POINT_LIGHTS,
            [camera_pos](const int a, const int b){
                    return camera_pos.distance_to(dynamic_cast<PointLight*>(ge.get_thing(a))->transform.position) < camera_pos.distance_to(dynamic_cast<PointLight*>(ge.get_thing(b))->transform.position);   // custom order
                }
            );
        }

        if (directional_lights.size() > MAX_NR_DIRECTIONAL_LIGHTS) {
            std::ranges::nth_element
            (directional_lights, directional_lights.begin() + MAX_NR_DIRECTIONAL_LIGHTS,
            [camera_pos](const int a, const int b){
                    return camera_pos.distance_to(dynamic_cast<PointLight*>(ge.get_thing(a))->transform.position) < camera_pos.distance_to(dynamic_cast<PointLight*>(ge.get_thing(b))->transform.position);   // custom order
                }
            );
        }
    }

    // FILL IN BUFFER DATA
    glBindBuffer(GL_UNIFORM_BUFFER, lights_ubo);
    // point lights
    for (size_t i = 0; i < std::min(point_lights.size(), static_cast<size_t>(MAX_NR_POINT_LIGHTS)); i++) {
        const auto ptl = dynamic_cast<PointLight*>(ge.get_thing(point_lights[i]));

        unsigned int byte_offset = static_cast<unsigned int>(i) * PointLight::STRUCT_BYTE_SIZE;

        glm::vec3 plt_pos = ptl->transform.position.glm_vector();

        glBufferSubData(GL_UNIFORM_BUFFER, byte_offset, sizeof(float) * 3, &ptl->color);
        glBufferSubData(GL_UNIFORM_BUFFER, byte_offset + sizeof(float) * 3, sizeof(float), &ptl->intensity);
        glBufferSubData(GL_UNIFORM_BUFFER, byte_offset + sizeof(float) * 4, sizeof(float) * 3, &plt_pos);
    }
    // directional lights
    for (size_t i = 0; i < std::min(directional_lights.size(), static_cast<size_t>(MAX_NR_DIRECTIONAL_LIGHTS)); i++) {
        const auto dl = dynamic_cast<DirectionalLight*>(ge.get_thing(directional_lights[i]));

        unsigned int byte_offset = static_cast<unsigned int>(i) * DirectionalLight::STRUCT_BYTE_SIZE + MAX_NR_POINT_LIGHTS * PointLight::STRUCT_BYTE_SIZE;

        glBufferSubData(GL_UNIFORM_BUFFER, byte_offset, sizeof(float) * 3, &dl->color);
        glBufferSubData(GL_UNIFORM_BUFFER, byte_offset + sizeof(float) * 3, sizeof(float), &dl->intensity);
        glBufferSubData(GL_UNIFORM_BUFFER, byte_offset + sizeof(float) * 4, sizeof(float) * 3, &dl->direction);
    }
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

bool Lights::add_point_light(const int ge_ref_id) {
    if (light_overflow_action == CANCEL_NEW and point_lights.size() >= MAX_NR_POINT_LIGHTS) {
        std::cerr << "Failed to add point light, LIMIT REACHED" << std::endl;
        return false;
    }

    point_lights.push_back(ge_ref_id);
    return true;
}

void Lights::remove_point_light(const int ge_ref_id) {
    point_lights[std::ranges::find(point_lights, ge_ref_id) - point_lights.begin()] = point_lights.back();
    point_lights.pop_back();
}

bool Lights::add_directional_light(const int ge_ref_id) {
    if (light_overflow_action == CANCEL_NEW and directional_lights.size() >= MAX_NR_DIRECTIONAL_LIGHTS) {
        std::cerr << "Failed to add point light, LIMIT REACHED" << std::endl;
        return false;
    }

    directional_lights.push_back(ge_ref_id);
    return true;
}


void Lights::remove_directional_light(const int ge_ref_id) {
    directional_lights[std::ranges::find(directional_lights, ge_ref_id) - directional_lights.begin()] = directional_lights.back();
    directional_lights.pop_back();
}
