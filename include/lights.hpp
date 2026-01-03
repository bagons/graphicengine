#ifndef LIGHTS_H
#define LIGHTS_H

#include <vector>
#include "coordinates.h"

class PointLightThing;

class Lights {
    std::vector<int> point_lights;
public:
    unsigned int lights_ubo = -1;
    unsigned int max_rendered_point_lights = 0;

    explicit Lights(unsigned int _max_rendered_point_lights = 16);
    ~Lights();

    void init_ubo();
    void update_ubo(Position& camera_pos);

    void add_point_light(int ge_ref_id);
    void remove_point_light(int ge_ref_id);
};

#endif //LIGHTS_H
