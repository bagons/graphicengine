#version 330 core
#ifdef USE_BINDLESS
#extension GL_ARB_bindless_texture : enable
#endif

#ifdef USE_BINDLESS
#define SAMPLER_UNIFORM layout(bindless_sampler) uniform sampler2D
#else
#define SAMPLER_UNIFORM uniform sampler2D
#endif

#ifdef HAS_UV
in vec2 UV;
#endif
in vec3 NORMAL;
in vec3 FRAG_GLOBAL_POS;
in vec3 CAMERA_GLOBAL_POS;

#ifdef HAS_TANGENTS
in mat3 TBN;
#endif

/* <GRAPHIC ENGINE DEFAULT MATERIAL> */

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
    vec3 albedo_color;
// albedo texture
    vec2 albedo_texture_scale;
/*sampler2D specular_texture;
vec2 specular_texture_offset;
vec2 specular_texture_scale;*/
#ifdef HAS_TANGENTS
    vec2 normal_map_scale;
    float normal_map_strength;
    vec2 bump_map_scale;
    float bump_map_strength;
#endif
};

uniform Material material;
SAMPLER_UNIFORM albedo_texture;
#ifdef HAS_TANGENTS
SAMPLER_UNIFORM normal_map;
SAMPLER_UNIFORM bump_map;
#endif
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


out vec4 FragColor;

mat3 blinn_phong_lighting(){
    mat3 out_light = mat3(
    vec3(0.0f, 0.0f, 0.0f),
    vec3(0.0f, 0.0f, 0.0f),
    vec3(0.0f, 0.0f, 0.0f)
    );


    #ifdef HAS_TANGENTS
    vec3 norm_detail = mix(vec3(0.0, 0.0, 1.0), texture(normal_map, (UV * material.normal_map_scale)).rgb * 2.0 - 1.0, material.normal_map_strength);

    vec2 texel_size = 1.0 / vec2(textureSize(bump_map, 0));
    vec2 bump_map_uvs = UV * material.bump_map_scale;
    float height = texture(bump_map, bump_map_uvs).r;
    float height_dx = texture(bump_map, bump_map_uvs + vec2(texel_size.x, 0)).r - height;
    float height_dy = texture(bump_map, bump_map_uvs + vec2(0, texel_size.y)).r - height;

    //norm_detail += vec3(height_dx, height_dy, 0.0);
    //norm_detail += vec3(, 0.0, 0.0);

    vec3 norm = normalize(TBN * norm_detail);
    #endif

    #ifndef HAS_TANGENTS
    vec3 norm = normalize(NORMAL);
    #endif

    vec3 view_dir = normalize(CAMERA_GLOBAL_POS - FRAG_GLOBAL_POS);

    float shininess = 0.6f;

    #ifdef HAS_UVS
    //shininess = texture(material.specular_texture, (UV * material.specular_texture_scale) + material.specular_texture_offset).r * material.shininess;
    #endif

    for(int i = 0; i < NR_POINT_LIGHTS; i++){
        vec4 light_data = point_lights[i].light_data;
        vec3 dir_to_light = point_lights[i].position - FRAG_GLOBAL_POS;

        float dist_to_light = dir_to_light.length();
        float attenuation = light_data.w / (1 + pow(dist_to_light, 2));

        vec3 dir = normalize(dir_to_light);
        float diff = max(dot(norm, dir), 0.0);
        out_light[1] += light_data.rgb * diff * attenuation;

        vec3 halfway_dir = normalize(dir + view_dir);
        //float spec = pow(max(dot(norm, halfway_dir), 0.0), 1.0f + material.shininess);
        float spec2 = pow(max(dot(norm, halfway_dir), 0.0), 1.0f + shininess);
        //out_light[2] += spec * light_data.rgb * attenuation;
        out_light[2] += spec2 * light_data.rgb * attenuation;
    }

    for(int i = 0; i < NR_DIRECTIONAL_LIGHTS; i++){
        vec4 light_data = directional_lights[i].light_data;
        /// diffuse
        vec3 dir = -normalize(directional_lights[i].direction);
        float diff = max(dot(norm, dir), 0.0);
        out_light[1] += light_data.rgb * diff;

        // Blinn-Phong specular
        vec3 halfway_dir = normalize(dir + view_dir);
        //float spec = pow(max(dot(norm, halfway_dir), 0.0), 1.0f + material.shininess);
        float spec2 = pow(max(dot(norm, halfway_dir), 0.0), 1.0f + shininess);
        //out_light[2] += light_data.rgb * spec;
        out_light[2] += light_data.rgb * spec2;
    }

    for (int i = 0; i < NR_SPOT_LIGHTS; i++){
        vec4 light_data = spot_lights[i].light_data;
        vec3 dir_to_light = normalize(spot_lights[i].position - FRAG_GLOBAL_POS);
        float theta = dot(dir_to_light, normalize(-spot_lights[i].direction));

        float attenuation = min(max(theta - spot_lights[i].cut_off, 0.0) * 50, 1.0);
        float diff = max(dot(norm, dir_to_light), 0.0);

        out_light[1] += light_data.rgb * diff * attenuation;

        // Blinn-Phong specular
        vec3 halfway_dir = normalize(dir_to_light + view_dir);
        float spec2 = pow(max(dot(norm, halfway_dir), 0.0), 1.0f + shininess);
        out_light[2] += light_data.rgb * spec2 * attenuation;
    }

    return out_light;
}

void main() {
    vec4 result = vec4(0.0, 0.0, 0.0, 1.0);

    mat3 lighting = blinn_phong_lighting();

    result.xyz = (BASE_AMBINET_LIGHT * material.ambient) + (lighting[1] * material.diffuse) + (lighting[2] * material.specular);
    result.xyz *= material.albedo_color;

    #ifdef HAS_UV
    result *= texture(albedo_texture, (UV * material.albedo_texture_scale));

    if (result.a < 0.1){
        discard;
    }
    #endif
    FragColor = result;
}
