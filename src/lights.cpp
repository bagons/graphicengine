#include "lights.hpp"
#include <algorithm>

#include "graphicengine.hpp"
#include "glad/glad.h"
#include "things.hpp"

Lights::Lights(unsigned int _max_rendered_point_lights) : max_rendered_point_lights(_max_rendered_point_lights) {

}

Lights::~Lights() {
    glDeleteBuffers(1, &lights_ubo);
}

void Lights::init_ubo() {
    unsigned int buffer_size = PointLightThing::size_on_gpu * max_rendered_point_lights;

    // create a Uniform Buffer for light system
    glGenBuffers(1, &lights_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, lights_ubo);
    glBufferData(GL_UNIFORM_BUFFER, buffer_size, nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, 1, lights_ubo, 0, buffer_size);
}

void Lights::update_ubo(Position& camera_pos) {
    // SORT LIGHTS BY PROXIMITY IF ABOVE LIGHT LIMIT
    if (point_lights.size() > max_rendered_point_lights) {
        /*std::ranges::nth_element(point_lights, point_lights.begin() + max_rendered_point_lights,
            [camera_pos](geRef<PointLightThing>& a, geRef<PointLightThing>& b)
            {
                return camera_pos.distance_to(a->transform.position) < camera_pos.distance_to(b->transform.position);   // custom order
            }
            );*/
    }

    // Fill in buffer data
    glBindBuffer(GL_UNIFORM_BUFFER, lights_ubo);
    for (size_t i = 0; i < std::min(point_lights.size(), static_cast<size_t>(max_rendered_point_lights)); i++) {
        const auto ptl = dynamic_cast<PointLightThing*>(ge.get_thing(point_lights[i]));
        glBufferSubData(GL_UNIFORM_BUFFER, static_cast<unsigned int>(i) * PointLightThing::size_on_gpu, sizeof(float), &ptl->intensity);
        glBufferSubData(GL_UNIFORM_BUFFER, static_cast<unsigned int>(i) * PointLightThing::size_on_gpu + 4, sizeof(glm::vec4), &ptl->color);
    }
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

