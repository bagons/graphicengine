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

uniform vec3 light_pos = vec3(0.0, 0.0, 0.0);
uniform vec3 light_color = vec3(1.0, 1.0, 1.0);

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;

    bool has_albedo;
    sampler2D albedo_texture;
};

uniform Material material;

out vec4 FragColor;

void main() {
    // ambient
    vec3 ambient = light_color * material.ambient;
  	
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
        
    vec3 result = ambient + diffuse + specular;

#ifdef HAS_UV
    result = material.has_albedo ? texture(material.albedo_texture, UV).xyz : vec3(0.0, 1.0, 0.0);
#endif

    FragColor = vec4(result, 1.0);
}