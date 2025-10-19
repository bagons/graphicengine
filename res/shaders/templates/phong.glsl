#version 330 core
#extension GL_ARB_bindless_texture : require

>u in vec2 UV;
in vec3 NORMAL;
in vec3 FRAG_GLOBAL_POS;
in vec3 CAMERA_GLOBAL_POS;

uniform vec3 light_pos;
uniform vec3 light_color;
uniform vec3 object_color;

out vec4 FragColor;

float specular_strength = 1;

void main() {
    vec3 ambient_color = vec3(0.2f);
    vec3 norm = normalize(NORMAL);

    vec3 light_dir = normalize(light_pos - FRAG_GLOBAL_POS);

    vec3 to_cam_dir = normalize(FRAG_GLOBAL_POS - CAMERA_GLOBAL_POS);
    vec3 reflect_dir = reflect(-light_dir, norm);

    float diff = dot(norm, light_dir);
    float spec = pow(max(dot(to_cam_dir, reflect_dir), 0.0), 256) * specular_strength;

    vec3 lighting = light_color * (vec3(diff) + vec3(spec) + ambient_color);

    //FragColor = vec4(vec3(diff) * light_color , 1.0f);
    FragColor = vec4(object_color * lighting, 1.0f);
}