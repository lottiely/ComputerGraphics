#version 120


// per-fragment interpolated values from the vertex shader
varying vec3 fN;  		//object normals
varying vec3 fL, fL2, fL3;   //light 1,2,3 etc...
varying vec3 fE;  		//object xyz
varying vec3 fR;
varying  vec2 texCoord;
varying vec2 texCoord2;

uniform  vec4  color; 

uniform sampler2D texture;
uniform sampler2D texture2;
uniform vec4 AmbientProduct2, DiffuseProduct2, SpecularProduct2; //stationary Light
uniform vec4 AmbientProduct3, DiffuseProduct3, SpecularProduct3; //spinning Light
uniform vec4 AmbientProduct4, DiffuseProduct4, SpecularProduct4; //camera Light

//out vec4 fColor;

uniform float Shininess;

void main() 
{ 
	vec4 totalAmbient;
	vec4 totalDiffuse;
	vec4 totalSpecular;
	vec3 H;
    float Kd;
    float Ks;
	
    // Normalize the input lighting vectors
    vec3 nfN = normalize(fN);
    vec3 nfE = normalize(fE);
    vec3 nfL = normalize(fL);
	vec3 nfL2 = normalize(fL2);
	vec3 nfL3 = normalize(fL3);
    vec3 nfR = normalize(fR);
	
	// Placing in arrays for easier calculations
	vec3 lightPositions[3];
	lightPositions[0] = nfL;
	lightPositions[1] = nfL2;
	lightPositions[2] = nfL3;
	
	vec4 ambientProducts[3];
	ambientProducts[0] = AmbientProduct2;
	ambientProducts[1] = AmbientProduct3;
	ambientProducts[2] = AmbientProduct4;
	
	vec4 diffuseProducts[3];
    diffuseProducts[0] = DiffuseProduct2;
    diffuseProducts[1] = DiffuseProduct3;
	diffuseProducts[2] = DiffuseProduct4;
    
    vec4 specularProducts[3];
    specularProducts[0] = SpecularProduct2;
    specularProducts[1] = SpecularProduct3;
	specularProducts[2] = SpecularProduct4;
	
	for(int i = 0; i<2;i++){
	
		H = normalize( lightPositions[i] + nfE );
	
		totalAmbient += ambientProducts[i];
		
		Kd = max(dot(lightPositions[i], nfN), 0.0);
        totalDiffuse += Kd*diffuseProducts[i];
		
        Ks = pow(max(dot(nfN, H), 0.0), Shininess);
        totalSpecular  += Ks*specularProducts[i];
		// discard the specular highlight if the light's behind the vertex
		if( dot(lightPositions[i], nfN) < 0.0 ) {
			totalSpecular = vec4(0.0, 0.0, 0.0, 1.0);
		}
	}
	
	
	//The last one is separated so that the light 
	//will follow the camera
	H = normalize( lightPositions[2] + nfE );
	
	totalAmbient += ambientProducts[2];
		
	Kd = max(dot(lightPositions[2], nfN), 0.0);
    totalDiffuse += Kd*diffuseProducts[2];
        
    Ks = pow(max(dot(nfR, nfE), 0.0), Shininess);
    totalSpecular  += Ks*specularProducts[2];
		
	vec4 diffusePlusAmbient = totalAmbient+totalDiffuse;
	gl_FragColor = diffusePlusAmbient*texture2D(texture, texCoord) + diffusePlusAmbient*texture2D(texture2, texCoord2)+  totalSpecular + color;
    gl_FragColor.a = 1.0;
} 

