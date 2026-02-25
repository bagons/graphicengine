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

/* <GRAPHIC ENGINE DEFAULT MATERIAL> */
struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;

    bool has_albedo_texture;
    vec3 albedo_color;
    sampler2D albedo_texture;
};

uniform Material material;
/* </GRAPHIC ENGINE DEFAULT MATERIAL> */

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
    vec3 BASE_AMBINET_LIGHT;
    PointLight point_lights[NR_POINT_LIGHTS];
    DirectionalLight directional_lights[NR_DIRECTIONAL_LIGHTS];
};

/* </GRAPHIC ENGINE TEMPLATE CODE> */

/* <GRAPHIC ENGINE DEFAULT LIGHT FUNCTION> */
mat3 light() {
    mat3 out_lights = mat3(
    BASE_AMBINET_LIGHT,
    vec3(0.0f, 0.0f, 0.0f),
    vec3(0.0f, 0.0f, 0.0f)
    );

    vec3 norm = normalize(NORMAL);

    vec3 view_dir = normalize(CAMERA_GLOBAL_POS - FRAG_GLOBAL_POS);

    // point lights
    for(int i = 0; i < NR_POINT_LIGHTS; i++){
        vec4 light_data = point_lights[i].light_data;
        vec3 dir_to_light = point_lights[i].position - FRAG_GLOBAL_POS;

        // intensity
        float dist_to_light = dir_to_light.length();
        float attenuation = light_data.w == 0 ? 0 : 1/(min(1/log(light_data.w*10), 1.0) + (1.2/light_data.w) * dist_to_light + (30.0 / pow(light_data.w, 2)) * pow(dist_to_light, 2));

        // Diffuse
        vec3 dir = normalize(dir_to_light);
        float diff = max(dot(norm, dir), 0.0);
        out_lights[1] += light_data.rgb * diff * attenuation;

        // Blinn-Phong specular
        vec3 halfway_dir = normalize(dir + view_dir);
        float spec = pow(max(dot(norm, halfway_dir), 0.0), material.shininess);
        out_lights[2] += light_data.rgb * spec * attenuation;
    }
    // directional lights
    for(int i = 0; i < NR_DIRECTIONAL_LIGHTS; i++){
        vec4 light_data = directional_lights[i].light_data;
        /// diffuse
        vec3 dir = -normalize(directional_lights[i].direction);
        float diff = max(dot(norm, dir), 0.0);
        out_lights[1] += light_data.rgb * diff;

        // Blinn-Phong specular
        vec3 halfway_dir = normalize(dir + view_dir);
        float spec = pow(max(dot(norm, halfway_dir), 0.0), material.shininess);
        out_lights[2] += light_data.rgb * spec;
    }

    return out_lights;
}
/* </GRAPHIC ENGINE DEFAULT LIGHT FUNCTION> */

out vec4 FragColor;


void main() {
    mat3 lighting = light();
    // ambient
    vec3 ambient = lighting[0] * material.ambient;
    // diffuse
    vec3 diffuse = lighting[1] * material.diffuse;

    // specular
    vec3 specular = lighting[2] * material.specular;
    //specular = vec3(0.0, 0.0, 0.0);

    vec3 result = ambient + diffuse + specular;

    result *= material.albedo_color;
    #ifdef HAS_UV
    result *= material.has_albedo_texture ? texture(material.albedo_texture, UV).xyz : vec3(1.0, 1.0, 1.0);
    #endif

    FragColor = vec4(result, 1.0);
}