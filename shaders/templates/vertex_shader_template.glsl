#version 330 core

layout (location = 0) in vec3 VERTEX_POS;
>u layout (location = 1) in vec2 TEXTURE_COORDS;
!u>n layout (location = 1) in vec3 NORMALS;
>u>n layout (location = 2) in vec3 NORMALS;

layout (std140) uniform CamMats
{
    mat4 projection;
    mat4 view;
};

uniform mat4 transform;

out vec3 FRAG_GLOBAL_POS;
out vec3 CAMERA_GLOBAL_POS;

>u out vec2 UV;
>n out vec3 NORMAL;

void main(){
    gl_Position = projection * view * transform * vec4(VERTEX_POS, 1.0);
    FRAG_GLOBAL_POS = vec3(transform * vec4(VERTEX_POS, 1.0));
    CAMERA_GLOBAL_POS = -view[3].xyz;
>u    UV = TEXTURE_COORDS;
>n    NORMAL = NORMALS;
}