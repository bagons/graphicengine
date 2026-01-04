#include "lights.hpp"
#include <algorithm>
#include "graphicengine.hpp"
#include "glad/glad.h"
#include "things.hpp"
#include <iostream>


PointLight::PointLight(const glm::vec3 color, const float intensity, const bool is_updatable) : SpatialThing(false, is_updatable),
color(color), intensity(intensity) {

}

Lights::Lights(unsigned int MAX_NR_POINT_LIGHTS, LightOverflowAction light_overflow_action) : MAX_NR_POINT_LIGHTS(MAX_NR_POINT_LIGHTS), light_overflow_action(light_overflow_action) { }


Lights::~Lights() {
    glDeleteBuffers(1, &lights_ubo);
}

void Lights::init_central_light_system() {
    const unsigned int buffer_size = PointLight::STRUCT_BYTE_SIZE * MAX_NR_POINT_LIGHTS;

    // create a Uniform Buffer for light system
    glGenBuffers(1, &lights_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, lights_ubo);
    glBufferData(GL_UNIFORM_BUFFER, buffer_size, nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, 1, lights_ubo, 0, buffer_size);
}




void Lights::update(Position& camera_pos) {
    // SORT LIGHTS BY PROXIMITY IF ABOVE LIGHT LIMIT
    if (light_overflow_action == SORT_BY_PROXIMITY and point_lights.size() > MAX_NR_POINT_LIGHTS) {
        std::ranges::nth_element
        (point_lights, point_lights.begin() + MAX_NR_POINT_LIGHTS,
        [camera_pos](const int a, const int b){
                return camera_pos.distance_to(dynamic_cast<PointLight*>(ge.get_thing(a))->transform.position) < camera_pos.distance_to(dynamic_cast<PointLight*>(ge.get_thing(b))->transform.position);   // custom order
            }
        );
    }

    // FILL IN BUFFER DATA
    glBindBuffer(GL_UNIFORM_BUFFER, lights_ubo);
    // point lights
    for (size_t i = 0; i < std::min(point_lights.size(), static_cast<size_t>(MAX_NR_POINT_LIGHTS)); i++) {
        const auto ptl = dynamic_cast<PointLight*>(ge.get_thing(point_lights[i]));

        unsigned int byte_offset = static_cast<unsigned int>(i) * PointLight::STRUCT_BYTE_SIZE;

        glBufferSubData(GL_UNIFORM_BUFFER, byte_offset, sizeof(glm::vec3), &ptl->color);
        glBufferSubData(GL_UNIFORM_BUFFER, byte_offset + sizeof(glm::vec3), sizeof(float), &ptl->intensity);
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
