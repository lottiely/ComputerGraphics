#version 150

in  vec4 vPosition;
uniform mat4 projection;
uniform mat4 modelview;
void main() 
{ 
  gl_Position = projection*modelview*vPosition;
}