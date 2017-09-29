#version 120

attribute  vec4 vPosition;

varying    vec4 color;

uniform    vec4 vColor;
uniform mat4 ModelView;
uniform mat4 Projection;

void main() 
{
    color = vColor;
    gl_Position = Projection*ModelView*vPosition;
}