#version 120

in   vec4 vPosition;
in   vec3 vNormal;
attribute vec2 vTexCoord;

uniform mat4 ModelView;
uniform mat4 NormalView;
uniform mat4 Camera;
uniform vec4 LightPosition, LightPosition2, LightPosition3;
uniform mat4 Projection;

// output values that will be interpretated per-fragment
varying vec2 texCoord;
varying vec3 fN; //per fragment normal in world coords
varying vec3 fE; //per fragment viewing direction in camera coords
varying vec3 fL, fL2, fL3; //per fragment light direction in world coords
varying vec3 fR; //Per fragment perfect reflector in camera coords



void main()
{
    fN = normalize((NormalView*vec4(vNormal,0.0)).xyz);
    fE = -(ModelView*vPosition).xyz;
    fL = normalize(LightPosition.xyz); //distant light 
	fL2 = normalize(LightPosition2.xyz); //spinning light 
	fL3 = -normalize(LightPosition3.xyz); //camera light
	fR = (Camera*vec4(reflect(fL3, fN),0.0)).xyz;
	
    gl_Position = Projection*ModelView*vPosition;
	texCoord    = vTexCoord;
}
