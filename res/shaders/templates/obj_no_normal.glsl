#version 330 core
#ifdef USE_BINDLESS
#extension GL_ARB_bindless_texture : require
#endif

>u in vec2 UV;
in vec3 NORMAL;
in vec3 FRAG_GLOBAL_POS;
in vec3 CAMERA_GLOBAL_POS;

uniform vec3 light_pos = vec3(0.0, 0.0, 0.0);
uniform vec3 light_color = vec3(1.0, 1.0, 1.0);

struct Material {
    vec3 diffuse;

    bool has_albedo;
    sampler2D albedo_texture;
};

uniform Material material;

out vec4 FragColor;

void main() {
    vec3 diffuse = light_color * material.diffuse;

>u  diffuse *= material.has_albedo ? texture(material.albedo_texture, UV).xyz : vec3(1.0);

    FragColor = vec4(diffuse, 1.0);
}