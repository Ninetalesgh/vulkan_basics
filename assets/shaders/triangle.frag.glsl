#version 450

layout(location = 0) out vec4 outputColor;

void main()
{
    
  vec4 c = gl_FragCoord; //top left to bottom right
  outputColor = vec4(c.x/1024,c.y/768,0,1.0);
}