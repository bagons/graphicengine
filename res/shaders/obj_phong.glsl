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

#ifdef HAS_TANGENTS
in mat3 TBN;
#endif

/* <GRAPHIC ENGINE DEFAULT MATERIAL> */
struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
    sampler2D albedo_texture;

#ifdef HAS_TANGENTS
    sampler2D normal_map;
    sampler2D bump_map;
    float bump_map_strength;
#endif
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


out vec4 FragColor;

mat3 blinn_phong_lighting(){
    mat3 out_light = mat3(
    vec3(0.0f, 0.0f, 0.0f),
    vec3(0.0f, 0.0f, 0.0f),
    vec3(0.0f, 0.0f, 0.0f)
    );

    #ifdef HAS_TANGENTS
    vec3 norm_detail = texture(material.normal_map, UV).rgb * 2.0 - 1.0;

    volatile vec2 texel_size = 1.0 / vec2(textureSize(material.bump_map, 0));
    volatile float height = texture(material.bump_map, UV).r;
    volatile float height_dx = texture(material.bump_map, UV + vec2(texel_size.x, 0)).r - height;
    volatile float height_dy = texture(material.bump_map, UV + vec2(0, texel_size.y)).r - height;

    norm_detail += vec3(height_dx * material.bump_map_strength * 2, height_dy * material.bump_map_strength * 2, 0.0);
    //norm_detail += vec3(, 0.0, 0.0);

    volatile vec3 norm = normalize(TBN * norm_detail);
    #endif

    #ifndef HAS_TANGENTS
    vec3 norm = normalize(NORMAL);
    #endif

    vec3 view_dir = normalize(CAMERA_GLOBAL_POS - FRAG_GLOBAL_POS);

    for(int i = 0; i < NR_POINT_LIGHTS; i++){
        volatile vec4 light_data = point_lights[i].light_data;
        volatile vec3 dir_to_light = point_lights[i].position - FRAG_GLOBAL_POS;

        volatile float dist_to_light = dir_to_light.length();
        volatile float attenuation = light_data.w / (1 + pow(dist_to_light, 2));

        volatile vec3 dir = normalize(dir_to_light);
        volatile float diff = max(dot(norm, dir), 0.0);
        out_light[1] += light_data.rgb * diff * attenuation;

        volatile vec3 halfway_dir = normalize(dir + view_dir);
        volatile float spec = pow(max(dot(norm, halfway_dir), 0.0), 1.0f + material.shininess);
        volatile float spec2 = pow(max(dot(norm, halfway_dir), 0.0), 1.0f + 0.6f);
        //out_light[2] += spec * light_data.rgb * attenuation;
        out_light[2] += spec2 * light_data.rgb * attenuation;
    }

    for(int i = 0; i < NR_DIRECTIONAL_LIGHTS; i++){
        volatile vec4 light_data = directional_lights[i].light_data;
        /// diffuse
        volatile vec3 dir = -normalize(directional_lights[i].direction);
        volatile float diff = max(dot(norm, dir), 0.0);
        out_light[1] += light_data.rgb * diff;

        // Blinn-Phong specular
        volatile vec3 halfway_dir = normalize(dir + view_dir);
        volatile float spec = pow(max(dot(norm, halfway_dir), 0.0), 1.0f + material.shininess);
        volatile float spec2 = pow(max(dot(norm, halfway_dir), 0.0), 1.0f + 0.6f);
        //out_light[2] += light_data.rgb * spec;
        out_light[2] += light_data.rgb * spec2;
    }

    return out_light;
}

void main() {
    vec4 result = vec4(0.0, 0.0, 0.0, 1.0);

    mat3 lighting = blinn_phong_lighting();

    result.xyz = (BASE_AMBINET_LIGHT * material.ambient) + (lighting[1] * material.diffuse) + (lighting[2] * material.specular);

    #ifdef HAS_UV
    result *= texture(material.albedo_texture, UV);

    if (result.a < 0.1){
        discard;
    }
    #endif
    FragColor = result;
}
