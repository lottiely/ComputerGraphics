#version 150

in vec4 vPosition;
uniform   mat4 ModelView;
uniform   mat4 Projection;

out vec3 texCoord;

void main() {

    texCoord    = normalize(vPosition.xyz);
    
      
    gl_Position = Projection*ModelView*vPosition;
    
}