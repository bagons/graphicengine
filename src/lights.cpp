#include "lights.hpp"
#include <algorithm>
#include "graphicengine.hpp"
#include "glad/glad.h"
#include "things.hpp"
#include <iostream>


PointLight::PointLight(const Color color, const float intensity) : color(color), intensity(intensity) {

}

DirectionalLight::DirectionalLight(const Color color, const float intensity, const Vector3 &direction) :
color(color), intensity(intensity), direction(direction) {

}

Lights::Lights(unsigned int MAX_NR_POINT_LIGHTS, unsigned int MAX_NR_DIRECTIONAL_LIGHTS, LightOverflowAction light_overflow_action) :
MAX_NR_POINT_LIGHTS(MAX_NR_POINT_LIGHTS), MAX_NR_DIRECTIONAL_LIGHTS(MAX_NR_DIRECTIONAL_LIGHTS), light_overflow_action(light_overflow_action), ambient_light(Color::BLACK) { }


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
    // ambient light
    glBufferSubData(GL_UNIFORM_BUFFER, 0, 4 * sizeof(float), &ambient_light);
    constexpr unsigned int BASE_OFFSET = 4 * sizeof(float);
    // point lights
    for (size_t i = 0; i < MAX_NR_POINT_LIGHTS; i++) {
        glm::vec3 plt_pos{0};
        glm::vec3 light_color{0};
        float intensity = 0;

        const unsigned int byte_offset = static_cast<unsigned int>(i) * PointLight::STRUCT_BYTE_SIZE + BASE_OFFSET;
        if (i < point_lights.size()) {
            const auto ptl = dynamic_cast<PointLight*>(ge.get_thing(point_lights[i]));

            plt_pos = ptl->transform.position.glm_vector();
            light_color = ptl->color.no_alpha().glm_vector();
            intensity = ptl->intensity;
            std::cout << "point light: " << i << " int: " << intensity << " z: " << plt_pos.z << " " << light_color.x << std::endl;
        }
        glBufferSubData(GL_UNIFORM_BUFFER, byte_offset, sizeof(float) * 3, &light_color);
        glBufferSubData(GL_UNIFORM_BUFFER, byte_offset + static_cast<unsigned int>(sizeof(float)) * 3, sizeof(float), &intensity);
        glBufferSubData(GL_UNIFORM_BUFFER, byte_offset + static_cast<unsigned int>(sizeof(float) * 4), sizeof(float) * 3, &plt_pos);
    }
    // directional lights
    for (size_t i = 0; i < MAX_NR_DIRECTIONAL_LIGHTS; i++) {
        glm::vec3 light_color{};
        glm::vec3 dir{};
        float intensity = 0;

        unsigned int byte_offset = static_cast<unsigned int>(i) * DirectionalLight::STRUCT_BYTE_SIZE + MAX_NR_POINT_LIGHTS * PointLight::STRUCT_BYTE_SIZE + BASE_OFFSET;
        if (i < directional_lights.size()) {
            auto dl = dynamic_cast<DirectionalLight*>(ge.get_thing(directional_lights[i]));


            light_color = dl->color.no_alpha().glm_vector();
            dir = dl->direction.glm_vector();
        }

        glBufferSubData(GL_UNIFORM_BUFFER, byte_offset, sizeof(float) * 3, &light_color);
        glBufferSubData(GL_UNIFORM_BUFFER, byte_offset +  static_cast<unsigned int>(sizeof(float)) * 3, sizeof(float), &intensity);
        glBufferSubData(GL_UNIFORM_BUFFER, byte_offset +  static_cast<unsigned int>(sizeof(float)) * 4, sizeof(float) * 3, &dir);
    }
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

bool Lights::add_point_light(const unsigned int ge_ref_id) {
    if (light_overflow_action == CANCEL_NEW and point_lights.size() >= MAX_NR_POINT_LIGHTS) {
        std::cerr << "ENGINE WARNING: Failed to add point light, LIMIT REACHED. Returning null geRef." << std::endl;
        return false;
    }

    point_lights.push_back(ge_ref_id);
    return true;
}

void Lights::remove_point_light(const unsigned int ge_ref_id) {
    point_lights[std::ranges::find(point_lights, ge_ref_id) - point_lights.begin()] = point_lights.back();
    point_lights.pop_back();
}

bool Lights::add_directional_light(const unsigned int ge_ref_id) {
    if (light_overflow_action == CANCEL_NEW and directional_lights.size() >= MAX_NR_DIRECTIONAL_LIGHTS) {
        std::cerr << "ENGINE WARNING: Failed to add point light, LIMIT REACHED. Returning null geRef." << std::endl;
        return false;
    }

    directional_lights.push_back(ge_ref_id);
    return true;
}


void Lights::remove_directional_light(const unsigned int ge_ref_id) {
    directional_lights[std::ranges::find(directional_lights, ge_ref_id) - directional_lights.begin()] = directional_lights.back();
    directional_lights.pop_back();
}
