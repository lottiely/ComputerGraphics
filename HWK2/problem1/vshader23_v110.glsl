#version 110

attribute  vec4 vPosition;

uniform  vec4 vColor;

uniform mat4 projmat;
uniform mat4 modelview;

varying    vec4 color;

void main() 
{ 
  gl_Position = projmat*modelview*vec4(vPosition.x, vPosition.y, vPosition.z, 1);
  color = vColor;
}