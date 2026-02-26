#version 330 core

layout (location = 0) in vec3 VERTEX_POS;
#ifdef HAS_UV
layout (location = 1) in vec2 TEXTURE_COORDS;
#endif

#if defined(HAS_NORMALS) && defined(HAS_UV)
layout (location = 2) in vec3 NORMALS;
#elif defined(HAS_NORMALS)
layout (location = 1) in vec3 NORMALS;
#endif

layout (std140) uniform MATRICES
{
    mat4 projection;
    mat4 view;
};

uniform mat4 transform;

out vec3 FRAG_GLOBAL_POS;
out vec3 CAMERA_GLOBAL_POS;

#ifdef HAS_UV
out vec2 UV;
#endif

#ifdef HAS_NORMALS
uniform mat3 normal_matrix;
out vec3 NORMAL;
#endif

void main(){
    gl_Position = projection * view * transform * vec4(VERTEX_POS, 1.0);
    FRAG_GLOBAL_POS = vec3(transform * vec4(VERTEX_POS, 1.0));
    CAMERA_GLOBAL_POS = -view[3].xyz;
#ifdef HAS_UV
    UV = TEXTURE_COORDS;
#endif

#ifdef HAS_NORMALS
    NORMAL = NORMALS;
    NORMAL = normal_matrix * NORMALS;
#endif
}