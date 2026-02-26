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

    bool has_albedo;
    sampler2D albedo_texture;
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

layout (std140) uniform LIGHTS
{
    vec3 BASE_AMBINET_LIGHT; // <- only this used
    PointLight point_lights[NR_POINT_LIGHTS];
    DirectionalLight directional_lights[NR_DIRECTIONAL_LIGHTS];
};

/* </GRAPHIC ENGINE TEMPLATE CODE> */

uniform Material material;

out vec4 FragColor;

void main() {
    vec3 diffuse = material.diffuse * BASE_AMBINET_LIGHT;

#ifdef HAS_UV
    diffuse *= material.has_albedo ? texture(material.albedo_texture, UV).xyz : vec3(1.0);
#endif

    FragColor = vec4(diffuse, 1.0);
}