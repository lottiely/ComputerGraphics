//CG George Kamberov
//
//A simple 3D scene:
//			A cube mesh and a wireframe line strip built using the pointers-to-vertices DS and 
//			the Draw Elements mechanism.
//		
//
//
//
// Interaction: 
//     
//      
//      Use the up/down direction (arrow)  keys to Move the camera back/forward  
//      (along the fixed viewing axis). 
//
// We use perspective projection.
//
// Shaders used: 
//  vshader30_TwoCubes_FullPipe.glsl
//  vshader6_2.glsl
//  fshader30_TwoCubes.glsl
//  fshader6_2.glsl
//

#include "Angel.h"
#include<assert.h>

///////CONSTANT/////////////////////
#ifndef M_PI
#define M_PI            3.14159265358979323846
#endif

#define X               0
#define Y               1
#define Z               2
#define SCALE_VECTOR    1.0
#define SCALE_ANGLE     1.0





void m_glewInitAndVersion(void);
void reshape(int width, int height);

GLuint program[2]; 
GLuint VAO[2];
GLuint buffers[3];

enum {cube_vertices, cube_indeces, wire_indeces};

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;



//----------------------------------------------------------------------------

/////Camera unv basis///////

Angel::vec4 n = vec4(0.0, 0.0, 1.0, 0.0);
Angel::vec4 v = vec4(0.0, 1.0, 0.0, 00);
Angel::vec4 u = vec4(1.0, 0.0, 0.0, 0.0);
Angel::vec4 eye = vec4(0.0, 0.0, 4.0, 1.0);

GLfloat theta = 0;



GLfloat step = 20;
GLfloat theta_y = 0;
int spinToggle = 0; 
GLfloat d=0.5;
int w; int h;


// Vertices of a unit cube centered at origin, sides aligned with axes
point4 vertices[8] = {
    point4( -0.5, -0.5,  0.5, 1.0 ),
    point4( -0.5,  0.5,  0.5, 1.0 ),
    point4(  0.5,  0.5,  0.5, 1.0 ),
    point4(  0.5, -0.5,  0.5, 1.0 ),
    point4( -0.5, -0.5, -0.5, 1.0 ),
    point4( -0.5,  0.5, -0.5, 1.0 ),
    point4(  0.5,  0.5, -0.5, 1.0 ),
    point4(  0.5, -0.5, -0.5, 1.0 )
};

// A generate 12 flat shaded triangles using pointers to vertices (from 8 // vertices)

GLubyte indices[]= {
1, 0, 3, 
1, 3, 2, 
2, 3, 7, 
2, 7, 6, 
3, 0, 4, 
3, 4, 7, 
6, 5, 1, 
6, 1, 2, 
4, 5, 6, 
4, 6, 7, 
5, 4, 0, 
5, 0, 1
};

GLubyte indices_wire[]= {
1, 0,
0, 3, 
3, 2, 
1, 2, 
3, 7, 
2, 6, 
4, 7,
4, 0,
6, 5,
5, 1,
4, 5, 
6, 7
};


// RGBA olors
color4 green_transparent = color4( 0.0, 1.0, 0.0, 0.1 ); 


//uniform variables locations
GLuint color_loc;
GLuint proj_loc;
GLuint model_view_loc;


//make sure you start with the default coordinate system
mat4 projmat = mat4(1.0);

mat4 modelviewStackTop = mat4(1.0);
mat4 modelviewStackBottom=  mat4(1.0);


//----------------------------------------------------------------------------

// OpenGL initialization
void
init()
{
	
/**
***  Create and initialize buffer objects
**/

	
	glGenBuffers( 3, buffers );
	//Vertex buffer for the vertex coordinates
	glBindBuffer( GL_ARRAY_BUFFER, buffers[cube_vertices] ); 
	glBufferData( GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW );

	//Elements buffer for the pointers
	 glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, buffers[cube_indeces] ); 
	 glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); 

	 glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, buffers[wire_indeces] ); 
	 glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(indices_wire), indices_wire, GL_STATIC_DRAW); 
     
	 
	 glGenVertexArrays(2, VAO);
	


// Load shaders and use the resulting shader programs
    program[0] = InitShader( "vshader30_TwoCubes_FullPipe.glsl", "fshader30_TwoCubes.glsl" ); 
	program[1] = InitShader( "vshader30_TwoCubes_FullPipe.glsl", "fshader6_2.glsl" ); 

    
	

	
   //VAO[0] the Cube 
	glUseProgram( program[0] );
	glBindVertexArray(VAO[0]);
	glBindBuffer( GL_ARRAY_BUFFER, buffers[cube_vertices] ); 
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[cube_indeces]);
	GLuint vPosition = glGetAttribLocation( program[0], "vPosition" );
	glEnableVertexAttribArray( vPosition );
	glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );
   	//done with this packet
    glBindVertexArray(0); 

	//VAO[1] the wireframe
	glUseProgram(program[1]);
	glBindVertexArray(VAO[1]);
	glBindBuffer( GL_ARRAY_BUFFER, buffers[cube_vertices] ); 
	GLuint vPosition_Wire = glGetAttribLocation( program[1], "vPosition" ); 
	glEnableVertexAttribArray( vPosition_Wire );
	glVertexAttribPointer( vPosition_Wire, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[wire_indeces]);
	//done with this packet
    glBindVertexArray(0);

	glUseProgram(0);

	glClearColor( 1.0, 1.0, 1.0, 0.0 ); 
	glClearDepth( 1.0 ); 
	glEnable( GL_DEPTH_TEST );
	glDepthFunc(GL_LEQUAL);
}

//----------------------------------------------------------------------------

void
display( void )
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
 	//Make sure you start with the Default Coordinate Systems
	projmat=mat4(1.0);
	modelviewStackTop = mat4(1.0);
	mat4 modelviewStackBottom =  modelviewStackTop;
	//Set up the camera optics
	projmat = projmat*Perspective(30,1.0,0.1,20.0); 
	//Position and orient the camera
	modelviewStackTop = modelviewStackTop*LookAt(eye, eye-n, v);
    modelviewStackTop = modelviewStackTop*Translate(0.15, 0.15, -2.5);
	//Spin the whole scene by pressing the X key
	modelviewStackTop = modelviewStackTop*RotateX(theta);
	


	modelviewStackBottom =  modelviewStackTop; //mvstack.push
	
	//Render wireframe
	glUseProgram(program[1]);
	proj_loc       = glGetUniformLocation(program[1], "projection");
	model_view_loc = glGetUniformLocation(program[1], "modelview");
	glUniformMatrix4fv(proj_loc, 1, GL_TRUE, projmat);
	glUniformMatrix4fv(model_view_loc, 1, GL_TRUE, modelviewStackTop);
	glBindVertexArray(VAO[1]);
	glDrawElements(GL_LINES, 24, GL_UNSIGNED_BYTE, BUFFER_OFFSET(0));
	
	modelviewStackTop = modelviewStackBottom; //mvstack.pop

	//Render surface mesh
    glUseProgram(program[0]);
	color_loc = glGetUniformLocation(program[0], "color");
	proj_loc       = glGetUniformLocation(program[0], "projection");
	model_view_loc = glGetUniformLocation(program[0], "modelview");
	glUniformMatrix4fv(proj_loc, 1, GL_TRUE, projmat);
	glUniformMatrix4fv(model_view_loc, 1, GL_TRUE, modelviewStackTop);
    glUniform4fv(color_loc, 1, green_transparent);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.0, 1.0);
	glBindVertexArray(VAO[0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, BUFFER_OFFSET(0));
	glDisable(GL_POLYGON_OFFSET_FILL);

	modelviewStackTop = modelviewStackBottom; //mvstack.pop



	glutSwapBuffers();

}

//----------------------------------------------------------------------------

void
keyboard( unsigned char key, int x, int y )
{
    switch( key ) {
	case 'x': {	theta += step;
		if (theta > 360.0) {
			theta -= 360.0;
		}
	}
			  break;
	case 'X': {	theta -= step;
		if (theta < -360.0) {
			theta += 360.0;
		}
	}
			  break;
	case 033:  // Escape key
	case 'q': case 'Q':
	    exit( EXIT_SUCCESS );
	    break;
	}
}

void specKey(int key, int x, int y)
{ 
switch( key ) {

case GLUT_KEY_UP: // MOVE FORWARD
    eye[0] -= SCALE_VECTOR * n[0];
    eye[1] -= SCALE_VECTOR * n[1];
    eye[2] -= SCALE_VECTOR * n[2];
    break;
  case GLUT_KEY_DOWN: // MOVE BACKWARD
    eye[0] += SCALE_VECTOR * n[0];
    eye[1] += SCALE_VECTOR * n[1];
    eye[2] += SCALE_VECTOR * n[2];
    break;
  case GLUT_KEY_LEFT: // MOVE LEFT
    eye[0] -= SCALE_VECTOR * u[0];
    eye[1] -= SCALE_VECTOR * u[1];
    eye[2] -= SCALE_VECTOR * u[2];
    break;
  case GLUT_KEY_RIGHT: // MOVE RIGHT
    eye[0] += SCALE_VECTOR * u[0];
    eye[1] += SCALE_VECTOR * u[1];
    eye[2] += SCALE_VECTOR * u[2];
    break;
  }
}

void idle(void){
  glutPostRedisplay();
}
//----------------------------------------------------------------------------

int main( int argc, char **argv )
{
    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
    glutInitWindowSize( 512, 512 );
	glutCreateWindow( "Pointers-to-Vertex, VAOs, Multiple Pipelines" );
	m_glewInitAndVersion();
	

    init();

	glutReshapeFunc(reshape);
    glutDisplayFunc( display );
    glutKeyboardFunc( keyboard );
	glutSpecialFunc( specKey );
	glutIdleFunc(idle);
	

    glutMainLoop();
    return 0;
}

void m_glewInitAndVersion(void)
{
   fprintf(stdout, "OpenGL Version: %s\n", glGetString(GL_VERSION));
   fprintf(stdout, "GLSL Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
   GLenum err = glewInit();
   if (GLEW_OK != err)
   {
   fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
   }
   fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
}

void reshape( int width, int height )
//the same objects are shown possibly distorted  
//original viewport is a square
{

	glViewport( 0, 0, (GLsizei) width, (GLsizei) height );

}
