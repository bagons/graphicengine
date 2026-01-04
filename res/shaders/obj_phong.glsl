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


/* <GRAPHIC ENGINE TEMPLATE CODE> */
struct PointLight{
  vec4 light_data;
  vec3 position;
};

layout (std140) uniform LIGHTS
{
    PointLight point_lights[NR_POINT_LIGHTS];
};

/* </GRAPHIC ENGINE TEMPLATE CODE> */

/* <GRAPHIC ENGINE DEFAULT LIGHT FUNCTION> */
vec3 light() {
    vec3 out_color = vec3(0.0, 0.0, 0.0);

    vec3 norm = normalize(NORMAL);

    // point lights
    for(int i = 0; i < NR_POINT_LIGHTS; i++){
        vec3 dir = normalize(point_lights[i].position - FRAG_GLOBAL_POS);
        float diff = max(dot(norm, dir), 0.0);
        out_color += point_lights[i].light_data.xyz * diff;
    }
    return out_color;
}
/* </GRAPHIC ENGINE DEFAULT LIGHT FUNCTION> */

/* <GRAPHIC ENGINE DEFAULT MATERIAL> */
struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;

    bool has_albedo;
    sampler2D albedo_texture;
};

uniform Material material;
/* </GRAPHIC ENGINE DEFAULT MATERIAL> */

out vec4 FragColor;


void main() {
    // ambient
    /*vec3 ambient = light_color * material.ambient;
  	
    // diffuse 
    vec3 norm = normalize(NORMAL);
    vec3 lightDir = normalize(light_pos - FRAG_GLOBAL_POS);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light_color * (diff * material.diffuse);
    
    // specular
    vec3 viewDir = normalize(CAMERA_GLOBAL_POS - FRAG_GLOBAL_POS);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light_color * (spec * material.specular);  
    */
    vec3 result = vec3(0.0,0.0,0.0);

#ifdef HAS_UV
    result = material.has_albedo ? texture(material.albedo_texture, UV).xyz : vec3(0.0, 1.0, 0.0);
    result *= light();
#endif

    FragColor = vec4(result, 1.0);
}