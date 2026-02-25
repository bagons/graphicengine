#ifndef LIGHTS_H
#define LIGHTS_H

#include <vector>
#include "coordinates.h"
#include "things.hpp"

/// A PointLight
class PointLight : public SpatialThing {
public:
    /// The size of the PointLight struct on the GPU
    static constexpr unsigned int STRUCT_BYTE_SIZE = 32;
    /// Light color
    Color color;
    /// Light intensity
    float intensity;
    /// PointLight constructor
    /// @param color light color
    /// @param intensity light intesity
    PointLight(Color color, float intensity);
};

/// A Directional (Sun) Light
class DirectionalLight : public Thing {
public:
    /// The size of the DirectionalLight struct on the GPU
    static constexpr unsigned int STRUCT_BYTE_SIZE = 32;
    /// Light color
    Color color;
    /// Light intensity
    float intensity;
    Vector3 direction;
    /// PointLight constructor
    /// @param color light color
    /// @param intensity light intesity
    /// @param direction Direction of the incoming light
    DirectionalLight(Color color, float intensity, const Vector3 &direction);
};

/// Central Light System Manager
class Lights {
    /// List of geRef ids of all PointLights
    std::vector<unsigned int> point_lights;
    /// List of geRef ids for all DirectionalLights
    std::vector<unsigned int> directional_lights;
    /// OpenGL ID of the Uniform Buffer Object
    unsigned int lights_ubo = -1;
public:
    /// Two types of Light Limit Overflow actions
    enum LightOverflowAction {
        CANCEL_NEW,
        SORT_BY_PROXIMITY
    };

    /// Maximum amount of rendered PointLights
    unsigned int MAX_NR_POINT_LIGHTS;
    /// Maximum amount of rendered DirectionalLights
    unsigned int MAX_NR_DIRECTIONAL_LIGHTS;

    /// The base ambient light color used by the "default material"
    Color ambient_light;

    /// Light overflow solution
    LightOverflowAction light_overflow_action;

    /// Constructor of Light Manager
    /// @param MAX_NR_POINT_LIGHTS Max amount of rendered PointLights
    /// @param MAX_NR_DIRECTIONAL_LIGHTS Max amount of rendered DirectionalLights
    /// @param light_overflow_action Light overflow solution
    explicit Lights(unsigned int MAX_NR_POINT_LIGHTS = 16, unsigned int MAX_NR_DIRECTIONAL_LIGHTS = 1, LightOverflowAction light_overflow_action = SORT_BY_PROXIMITY);

    /// Destroys light UBO
    ~Lights();

    /// Initializes central light system (creates buffer based on the amount of max rendered lights)
    void init_central_light_system();

    /// Update central light system based on the Cameras position (for SORT_BY_PROXIMITY solution)
    void update(Position& camera_pos);

    /// Add a PointLight to the Central Light System
    /// @note Engine does this automatically, no need to do so for the user
    /// @param ge_ref_id geRef ID of the PointLight entity
    bool add_point_light(unsigned int ge_ref_id);

    /// Remove a PointLight from the Central Light System
    /// @note Engine does this automatically, no need to do so for the user
    /// @param ge_ref_id geRef ID of the PointLight entity
    void remove_point_light(unsigned int ge_ref_id);

    /// Add a DirectionalLight to the Central Light System
    /// @note Engine does this automatically, no need to do so for the user
    /// @param ge_ref_id geRef ID of the DirectionalLight entity
    bool add_directional_light(unsigned int ge_ref_id);

    /// Remove a DirectionalLight to the Central Light System
    /// @note Engine does this automatically, no need to do so for the user
    /// @param ge_ref_id geRef ID of the DirectionalLight entity
    void remove_directional_light(unsigned int ge_ref_id);
};

#endif //LIGHTS_H
