vec4 vPInst;

attribute  vec4 vPosition;
attribute  vec4 vColor;
attribute   vec4 vShift;
varying vec4 color;


uniform mat4  ModelView;
uniform mat4 Projection;

void main()
{
    
    vPInst = vPosition;
    
    
    vPInst.x += vShift.x;
    vPInst.z += vShift.z;
    
    
    gl_Position = Projection*ModelView*vPInst;
    color = vColor;
}