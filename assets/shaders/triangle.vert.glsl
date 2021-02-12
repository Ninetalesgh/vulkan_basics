#version 450
#extension GL_KHR_vulkan_glsl : enable 

const vec3 vertices[] =
{
    vec3(-1,-1, 0),
    vec3( 1,-1, 0),
    vec3( 1, 1, 0),

    vec3( 1, 1, 0),
    vec3(-1, 1, 0),
    vec3(-1,-1, 0),
};

void main()
{
  gl_Position = vec4(vertices[gl_VertexIndex], 1.0);
}