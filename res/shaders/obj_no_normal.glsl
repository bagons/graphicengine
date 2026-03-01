#version 330 core
#ifdef USE_BINDLESS
#extension GL_ARB_bindless_texture : require
#endif

#ifdef HAS_UV
in vec2 UV;
#endif
in vec3 NORMAL;
in vec3 FRAG_GLOBAL_POS;
in vec3 CAMERA_GLOBAL_POS;

struct Material {
    vec3 diffuse;
#ifdef HAS_UV
    sampler2D albedo_texture;
#endif
};

/* <GRAPHIC ENGINE TEMPLATE CODE> */
struct PointLight{
    vec4 light_data;
    vec3 position;
};

struct DirectionalLight{
    vec4 light_data;
    vec3 direction;
};

struct SpotLight{
    vec4 light_data;
    float cut_off;
    vec3 position;
    vec3 direction;
};

layout (std140) uniform LIGHTS
{
    vec3 BASE_AMBINET_LIGHT;
    PointLight point_lights[NR_POINT_LIGHTS];
    DirectionalLight directional_lights[NR_DIRECTIONAL_LIGHTS];
    SpotLight spot_lights[NR_SPOT_LIGHTS];
};

/* </GRAPHIC ENGINE TEMPLATE CODE> */

uniform Material material;

out vec4 FragColor;

void main() {
    vec3 diffuse = material.diffuse * BASE_AMBINET_LIGHT;

#ifdef HAS_UV
    diffuse *= texture(material.albedo_texture, UV).xyz;
#endif

    FragColor = vec4(diffuse, 1.0);
}