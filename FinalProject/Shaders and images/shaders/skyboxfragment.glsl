#version 150
in  vec3 texCoord;
uniform samplerCube cubeMap;
void main() {
   
   gl_FragColor = texture(cubeMap, texCoord);
   gl_FragColor.a = 1.0;
   
}