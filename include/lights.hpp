#ifndef LIGHTS_H
#define LIGHTS_H

#include <vector>
#include "coordinates.h"
#include "things.hpp"

class PointLight : public SpatialThing {
public:
    static constexpr unsigned int STRUCT_BYTE_SIZE = 32;
    glm::vec3 color;
    float intensity;
    PointLight(glm::vec3 color, float intensity, bool is_updatable = false);
};

class Lights {
    std::vector<int> point_lights;
    unsigned int lights_ubo = -1;
public:
    enum LightOverflowAction {
        CANCEL_NEW,
        SORT_BY_PROXIMITY
    };

    unsigned int MAX_NR_POINT_LIGHTS = 0;
    LightOverflowAction light_overflow_action;

    explicit Lights(unsigned int MAX_NR_POINT_LIGHTS = 16, LightOverflowAction light_overflow_action = SORT_BY_PROXIMITY);
    ~Lights();

    void init_central_light_system();
    void update(Position& camera_pos);

    bool add_point_light(int ge_ref_id);
    void remove_point_light(int ge_ref_id);
};

#endif //LIGHTS_H
