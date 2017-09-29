#version 120

vec4 vPInst;

in   vec4 vPosition;
in   vec3 vNormal;
attribute vec2 vTexCoord;
attribute vec2 vTexCoord2;
attribute vec4 vShift;

attribute  vec4 vColor;
varying vec4 color;

uniform mat4 ModelView;
uniform mat4 NormalView;
uniform mat4 Camera;
uniform vec4 LightPosition, LightPosition2, LightPosition3;
uniform mat4 Projection;

// output values that will be interpretated per-fragment
varying vec2 texCoord;
varying vec2 texCoord2;
varying vec3 fN; //per fragment normal in world coords
varying vec3 fE; //per fragment viewing direction in camera coords
varying vec3 fL, fL2, fL3; //per fragment light direction in world coords
varying vec3 fR; //Per fragment perfect reflector in camera coords



void main()
{
    fN = normalize((NormalView*vec4(vNormal,0.0)).xyz);
    fE =  -(normalize(ModelView*vPosition).xyz);
    fL = (normalize(ModelView*LightPosition).xyz); //distant light 
	fR = (Camera*vec4(reflect(fL3, fN),0.0)).xyz;
	
	vPInst = vPosition;
	vPInst.x += vShift.x;
	vPInst.z += vShift.z;
	
    gl_Position = Projection*ModelView*vPInst;
	texCoord    = vTexCoord;
	texCoord2 = vTexCoord2;
	color = vColor;
}
