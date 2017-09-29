//Shanan Almario
//CSCE 385
//
//Problem 2 
//	 This code should show a cylindrical cannon with two wheels and an axle.
//	 Mouse interaction and keyboard interaction has been implemented to examine cannon.
//
//Interaction:
//	 Keyboard:
//		z/Z = roll the camera 
//		y/Y yaw the camera  
//		x/X pitch the camera 
//	 
//		up   Special Key = slide forward -- implemented
//		down Special Key = slide backward -- you must implement
//		SPACEBAR  return to default (initial) camera position and oreintation 
//
//	 Mouse:
//		 If the mouse is moved up, then the object is rotated around the x axis, clockwise in the y?z coordinate plane.
//		 If the mouse is moved horizontally to the right/left, then the object is rotated around the y axis, clockwise in the z ? x coordinate plane.
//
//Compiling:
//Shaders Used:
//  vshaderPerVertColor.glsl
//  fshaderStock.glsl
//


#include "Angel.h"
#include <assert.h>



void m_glewInitAndVersion(void);
void reshape(int width, int height);
void specKey(int key, int x, int y);
void keyboard( unsigned char key, int x, int y );

void drawBarrel(void);//barrel
void drawWheel1(void); //wheel1
void drawWheel2(void); //wheel2
void drawAxle(void);//axle

//  Define PI in the case it's not defined in the math header file
#ifndef PI
#  define PI  3.14159265358979323846
#endif

typedef Angel::vec4 point4;
typedef Angel::vec4 color4;

#define X               0
#define Y               1
#define Z               2
#define SCALE_VECTOR    1.0
#define SCALE_ANGLE     1.0

/////Camera unv basis///////

Angel::vec4 v = vec4(0.0, 1.0, 0.0, 00);
Angel::vec4 u = vec4(1.0, 0.0, 0.0, 0.0);
Angel::vec4 eye = vec4(0.0, 2.0, 3.5, 1.0);
Angel::vec4 n = Angel::normalize(vec4(0.0, 1.0, 2.0, 0.0));

int startx;
int starty;
int moving;

GLfloat theta = 0;
GLfloat thetaY = 0;

GLfloat step = 20;
GLfloat theta_y = 0;
int spinToggle = 0;
GLfloat d = 0.5;
int w; int h;

GLuint buffers[7];
// Create buffer objects

// Create a vertex array object
GLuint vao[4];

GLuint program[4];

GLuint vPosition;
GLuint vColor;


//------------------------------------
//uniform variables locations

GLuint color_loc;
GLuint proj_loc;
GLuint model_view_loc;

mat4 modelviewStackTop = mat4(1.0);
mat4 modelviewStackBottom = mat4(1.0);

//------------------------------------


int CylNumVertices = 600;

point4 cylinderData[600];
point4 wheelData[600];
point4 axleData[600];
color4 cylinderColor[600];
color4 wheelColorData[600];
color4 axleColorData[600];

class MatrixStack {
    int    _index;
    int    _size;
    mat4*  _matrices;

   public:
    MatrixStack( int numMatrices = 32 ):_index(0), _size(numMatrices)
        { _matrices = new mat4[numMatrices]; }

    ~MatrixStack()
	{ delete[]_matrices; }

    void push( const mat4& m ) {
        assert( _index + 1 < _size );
        _matrices[_index++] = m;
    }

    mat4& pop( void ) {
        assert( _index - 1 >= 0 );
        _index--;
        return _matrices[_index];
    }
};


MatrixStack  mvstack;
mat4         model_view=mat4(1.0);
mat4         projmat=mat4(1.0);  

//--------------------------------------

enum {
	barrelNum = 0,
	wheel1Num = 1,
	wheel2Num = 2,
	axleNum = 3,
    NumNodes,
    Quit
};

//-------SG DS-------------------------------------------------------------

struct Node {
    mat4  transform;
    void  (*render)( void );
    Node* sibling;
    Node* child;

    Node() :
	render(NULL), sibling(NULL), child(NULL) {}
    
    Node( mat4& m, void (*render)( void ), Node* sibling, Node* child ) :
	transform(m), render(render), sibling(sibling), child(child) {}
};

Node  nodes[NumNodes];


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

void traverse( Node* node )
{
    if ( node == NULL ) { return; }

    mvstack.push( model_view );

    model_view *= node->transform;
    node->render();

    if ( node->child ) { traverse( node->child ); }

    model_view = mvstack.pop();

    if ( node->sibling ) { traverse( node->sibling ); }
}

//----------------------------------------------------------------------------


void buildCylinder( ){
	GLfloat x,z,theta;
	int iMax=50, vertexCounter=0;
	//build black cylinder top
	for(int i=0;i<iMax;i++){
		theta= (2*PI/(float)iMax)*i; x=cos(theta); z=sin(theta);
		cylinderData[vertexCounter]=vec4(0,1.5,0,1.0); cylinderColor[vertexCounter]=vec4(0.0,0.0,0.0, 1.0);
		vertexCounter++;
		cylinderData[vertexCounter]=vec4(x,1.5,z,1.0); cylinderColor[vertexCounter]=vec4(0.0, 0.0, 0.0, 1.0);
		vertexCounter++;	

		theta= (2*PI/(float)iMax)*(i+1.0); x=cos(theta); z=sin(theta);
		cylinderData[vertexCounter]=vec4(x,1.5,z,1.0); cylinderColor[vertexCounter]=vec4(0.0, 0.0, 0.0, 1.0);
		vertexCounter++;
	}
	//build gray cylinder bottom
	for(int i=0;i<iMax;i++){
		theta= (2*PI/(float)iMax)*i; x=cos(theta); z=sin(theta);
		cylinderData[vertexCounter]=vec4(0,0,0,1.0); cylinderColor[vertexCounter]=vec4(0.5, 0.5, 0.5, 1.0);
		vertexCounter++;
		cylinderData[vertexCounter]=vec4(x,0,z,1.0); cylinderColor[vertexCounter]=vec4(0.5, 0.5, 0.5, 1.0);
		vertexCounter++;	
		theta= (2*PI/(float)iMax)*(i+1.0); x=cos(theta); z=sin(theta);
		cylinderData[vertexCounter]=vec4(x,0,z,1.0); cylinderColor[vertexCounter]=vec4(0.5, 0.5, 0.5, 1.0);
		vertexCounter++;
	}

	//build red cylinder sides
	for(int i=0;i<iMax;i++){
		theta= (2*PI/(float)iMax)*i; x=cos(theta); z=sin(theta);
		cylinderData[vertexCounter]=vec4(x,1.5,z,1.0); cylinderColor[vertexCounter]=vec4(1.0,0.0,0.0, 0.5);
		vertexCounter++;
		cylinderData[vertexCounter]=vec4(x,0,z,1.0); cylinderColor[vertexCounter]=vec4(1.0, 0.0, 0.0, 0.5);
		vertexCounter++;

		theta= (2*PI/(float)iMax)*(i+1.0); x=cos(theta); z=sin(theta);
		cylinderData[vertexCounter]=vec4(x,1.5,z,1.0); cylinderColor[vertexCounter]=vec4(1.0, 0.0, 0.0, 0.5);
		vertexCounter++;
		cylinderData[vertexCounter]=vec4(x,1.5,z,1.0); cylinderColor[vertexCounter]=vec4(1.0, 0.0, 0.0, 0.5);
		vertexCounter++;

		theta= (2*PI/(float)iMax)*i; x=cos(theta); z=sin(theta);
		cylinderData[vertexCounter]=vec4(x,0,z,1.0); cylinderColor[vertexCounter]=vec4(1.0, 0.0, 0.0, 0.5);
		vertexCounter++;

		theta= (2*PI/(float)iMax)*(i+1.0); x=cos(theta); z=sin(theta);
		cylinderData[vertexCounter]=vec4(x,0,z,1.0); cylinderColor[vertexCounter]=vec4(1.0, 0.0, 0.0, 0.5);
		vertexCounter++;

	}
}

void buildWheel() {
	GLfloat x, z, theta;
	int iMax = 50, vertexCounter = 0;
	//build gray cylinder top
	for (int i = 0; i<iMax; i++) {
		theta = (2 * PI / (float)iMax)*i; x = cos(theta); z = sin(theta);
		wheelData[vertexCounter] = vec4(0, 1, 0, 1.0); wheelColorData[vertexCounter] = vec4(0.5, 0.5, 0.5, 1.0);
		vertexCounter++;
		wheelData[vertexCounter] = vec4(x, 1, z, 1.0); wheelColorData[vertexCounter] = vec4(0.5, 0.5, 0.5, 1.0);
		vertexCounter++;

		theta = (2 * PI / (float)iMax)*(i + 1.0); x = cos(theta); z = sin(theta);
		wheelData[vertexCounter] = vec4(x, 1, z, 1.0); wheelColorData[vertexCounter] = vec4(0.5, 0.5, 0.5, 1.0);
		vertexCounter++;
	}
	//build gray cylinder bottom
	for (int i = 0; i<iMax; i++) {
		theta = (2 * PI / (float)iMax)*i; x = cos(theta); z = sin(theta);
		wheelData[vertexCounter] = vec4(0, 0, 0, 1.0); wheelColorData[vertexCounter] = vec4(0.5, 0.5, 0.5, 1.0);
		vertexCounter++;
		wheelData[vertexCounter] = vec4(x, 0, z, 1.0); wheelColorData[vertexCounter] = vec4(0.5, 0.5, 0.5, 1.0);
		vertexCounter++;
		theta = (2 * PI / (float)iMax)*(i + 1.0); x = cos(theta); z = sin(theta);
		wheelData[vertexCounter] = vec4(x, 0, z, 1.0); wheelColorData[vertexCounter] = vec4(0.5, 0.5, 0.5, 1.0);
		vertexCounter++;
	}

	//build black cylinder sides
	for (int i = 0; i<iMax; i++) {
		theta = (2 * PI / (float)iMax)*i; x = cos(theta); z = sin(theta);
		wheelData[vertexCounter] = vec4(x, 1, z, 1.0); wheelColorData[vertexCounter] = vec4(0.0, 0.0, 0.0, 0.5);
		vertexCounter++;
		wheelData[vertexCounter] = vec4(x, 0, z, 1.0); wheelColorData[vertexCounter] = vec4(0.0, 0.0, 0.0, 0.5);
		vertexCounter++;

		theta = (2 * PI / (float)iMax)*(i + 1.0); x = cos(theta); z = sin(theta);
		wheelData[vertexCounter] = vec4(x, 1, z, 1.0); wheelColorData[vertexCounter] = vec4(0.0, 0.0, 0.0, 0.5);
		vertexCounter++;
		wheelData[vertexCounter] = vec4(x, 1, z, 1.0); wheelColorData[vertexCounter] = vec4(0.0, 0.0, 0.0, 0.5);
		vertexCounter++;

		theta = (2 * PI / (float)iMax)*i; x = cos(theta); z = sin(theta);
		wheelData[vertexCounter] = vec4(x, 0, z, 1.0); wheelColorData[vertexCounter] = vec4(0.0, 0.0, 0.0, 0.5);
		vertexCounter++;

		theta = (2 * PI / (float)iMax)*(i + 1.0); x = cos(theta); z = sin(theta);
		wheelData[vertexCounter] = vec4(x, 0, z, 1.0); wheelColorData[vertexCounter] = vec4(0.0, 0.0, 0.0, 0.5);
		vertexCounter++;

	}
}

void buildAxle() {
	GLfloat x, z, theta;
	int iMax = 50, vertexCounter = 0;

	//uncapped cylinder
	//build brown cylinder sides
	for (int i = 0; i<iMax; i++) {
		theta = (2 * PI / (float)iMax)*i; x = cos(theta); z = sin(theta);
		axleData[vertexCounter] = vec4(x, 1, z, 1.0); axleColorData[vertexCounter] = vec4(0.6, 0.3, 0.0, 0.5);
		vertexCounter++;
		axleData[vertexCounter] = vec4(x, 0, z, 1.0); axleColorData[vertexCounter] = vec4(0.6, 0.3, 0.0, 0.5);
		vertexCounter++;

		theta = (2 * PI / (float)iMax)*(i + 1.0); x = cos(theta); z = sin(theta);
		axleData[vertexCounter] = vec4(x, 1, z, 1.0); axleColorData[vertexCounter] = vec4(0.6, 0.3, 0.0, 0.5);
		vertexCounter++;
		axleData[vertexCounter] = vec4(x, 1, z, 1.0); axleColorData[vertexCounter] = vec4(0.6, 0.3, 0.0, 0.5);
		vertexCounter++;

		theta = (2 * PI / (float)iMax)*i; x = cos(theta); z = sin(theta);
		axleData[vertexCounter] = vec4(x, 0, z, 1.0); axleColorData[vertexCounter] = vec4(0.6, 0.3, 0.0, 0.5);
		vertexCounter++;

		theta = (2 * PI / (float)iMax)*(i + 1.0); x = cos(theta); z = sin(theta);
		axleData[vertexCounter] = vec4(x, 0, z, 1.0); axleColorData[vertexCounter] = vec4(0.6, 0.3, 0.0, 0.5);
		vertexCounter++;
	}
}

/////////////////////////////////////
///Where the objects are scaled and rotated
/////////////////////////////////////
void initNodes( void )
{
    mat4  m;
    
    m = mat4(1.0)*RotateX(15)*Scale(0.5, 1.0, 0.5);
    nodes[barrelNum] = Node( m, drawBarrel, &nodes[wheel1Num], NULL );

	m = Translate(-0.6, 0.0, 0.0)*RotateX(90)*RotateZ(90)*Scale(0.5, 0.2, 0.5);
	nodes[wheel1Num] = Node(m, drawWheel1, &nodes[wheel2Num], NULL);

	m = Translate(0.6, 0.0, 0.0)*RotateX(180+90)*RotateZ(180+90)*Scale(0.5, 0.2, 0.5);
	nodes[wheel2Num] = Node(m, drawWheel2, &nodes[axleNum],NULL);

	m = Translate(0.75, 0.0, 0.0)*RotateX(90)*RotateZ(90)*Scale(0.1, 1.5, 0.1);
	nodes[axleNum] = Node(m, drawAxle, NULL, NULL);
}

void init(){
    
buildCylinder( );
buildWheel();
buildAxle();

glGenBuffers(7,buffers); 
glGenVertexArrays( 4, vao );

//barrel	
glBindBuffer( GL_ARRAY_BUFFER, buffers[0] );
glBufferData( GL_ARRAY_BUFFER, sizeof(cylinderData),  cylinderData, GL_STATIC_DRAW );

glBindBuffer( GL_ARRAY_BUFFER, buffers[1] );
glBufferData( GL_ARRAY_BUFFER, sizeof(cylinderColor),  cylinderColor, GL_STATIC_DRAW );

//wheel1 & wheel2
glBindBuffer(GL_ARRAY_BUFFER, buffers[2]);
glBufferData(GL_ARRAY_BUFFER, sizeof(wheelData), wheelData, GL_STATIC_DRAW);

glBindBuffer(GL_ARRAY_BUFFER, buffers[3]);
glBufferData(GL_ARRAY_BUFFER, sizeof(wheelColorData), wheelColorData, GL_STATIC_DRAW);

glBindBuffer(GL_ARRAY_BUFFER, buffers[4]);
glBufferData(GL_ARRAY_BUFFER, sizeof(wheelData), wheelData, GL_STATIC_DRAW);

//axle 
glBindBuffer(GL_ARRAY_BUFFER, buffers[5]);
glBufferData(GL_ARRAY_BUFFER, sizeof(axleData), axleData, GL_STATIC_DRAW);

glBindBuffer(GL_ARRAY_BUFFER, buffers[6]);
glBufferData(GL_ARRAY_BUFFER, sizeof(axleColorData), axleColorData, GL_STATIC_DRAW);

// Load shaders and use the resulting shader programs
program[barrelNum] = InitShader( "vshaderPerVertColor.glsl", "fshaderStock.glsl" );
program[wheel1Num] = InitShader( "vshaderPerVertColor.glsl", "fshaderStock.glsl" );
program[wheel2Num] = InitShader("vshaderPerVertColor.glsl", "fshaderStock.glsl");
program[axleNum] = InitShader("vshaderPerVertColor.glsl", "fshaderStock.glsl");

//barrel
glBindVertexArray( vao[0] );

glBindBuffer( GL_ARRAY_BUFFER, buffers[0] );
vPosition = glGetAttribLocation( program[barrelNum], "vPosition" );
glEnableVertexAttribArray( vPosition );
glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );

glBindBuffer( GL_ARRAY_BUFFER, buffers[1] );
vColor = glGetAttribLocation( program[barrelNum], "vColor" );
glEnableVertexAttribArray( vColor );
glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );

//wheel1
glBindVertexArray(vao[1]);

glUseProgram(program[wheel1Num]);
glBindBuffer(GL_ARRAY_BUFFER, buffers[2]);
vPosition = glGetAttribLocation(program[wheel1Num], "vPosition");
glEnableVertexAttribArray(vPosition);
glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

glBindBuffer(GL_ARRAY_BUFFER, buffers[3]);
vColor = glGetAttribLocation(program[wheel1Num], "vColor");
glEnableVertexAttribArray(vColor);
glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

glBindVertexArray(0);

//wheel2
glBindVertexArray(vao[2]);

glUseProgram(program[wheel2Num]);
glBindBuffer(GL_ARRAY_BUFFER, buffers[4]);
vPosition = glGetAttribLocation(program[wheel2Num], "vPosition");
glEnableVertexAttribArray(vPosition);
glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

glBindBuffer(GL_ARRAY_BUFFER, buffers[3]);
vColor = glGetAttribLocation(program[wheel2Num], "vColor");
glEnableVertexAttribArray(vColor);
glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

glBindVertexArray(0);

//axle
glBindVertexArray(vao[3]);

glUseProgram(program[axleNum]);
glBindBuffer(GL_ARRAY_BUFFER, buffers[5]);
vPosition = glGetAttribLocation(program[axleNum], "vPosition");
glEnableVertexAttribArray(vPosition);
glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

glBindBuffer(GL_ARRAY_BUFFER, buffers[6]);
vColor = glGetAttribLocation(program[axleNum], "vColor");
glEnableVertexAttribArray(vColor);
glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

glBindVertexArray(0);

// Initialize tree
initNodes();

glClearColor( 1.0, 1.0, 1.0, 0.0 ); 
glClearDepth( 1.0 ); 
glEnable( GL_DEPTH_TEST );
glDepthFunc(GL_LEQUAL);
glPolygonMode(GL_FRONT, GL_FILL);

}

void display( void )
{
	 glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	 	
	 projmat = Perspective(90, 1.0, 1.0, 5.0);

	 model_view = LookAt(eye, eye-n, v); 
	 model_view = model_view*RotateX(theta);
	 model_view = model_view*RotateY(thetaY);

	 traverse( &nodes[barrelNum]);

     glutSwapBuffers();
}


void idle(void)
{
	glutPostRedisplay();
}

//////////////////////////////
//Mouse Functions
//////////////////////////////
static void
mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_DOWN) {
			startx = x;
			starty = y;
			moving = 1;
		}
		if (state == GLUT_UP) {
			moving = 0;
		}
	}
}

static void
motion(int x, int y) {
	if (moving) {
		thetaY = thetaY + (x - startx);
		theta = theta + (y - starty);
		startx = x;
		starty = y;
		glutPostRedisplay();
	}
}

//----------------------------------------------------------------------------

int
main( int argc, char **argv )
{
    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH );
    glutInitWindowSize( 512, 512 );
    glutCreateWindow( "Scene Graph and a Flying Camera Stub" );

	glutMouseFunc(mouse);
	glutMotionFunc(motion);

	glutDisplayFunc(display); 
	glutReshapeFunc(reshape);
	glutKeyboardFunc( keyboard );
	glutSpecialFunc( specKey );
	glutIdleFunc(idle);

    glewInit();

	init();

    glutMainLoop();
    return 0;
}


void keyboard( unsigned char key, int x, int y )
{
  GLfloat xt, yt, zt;
  GLfloat cosine, sine;

  //prees  spacebar to  return to default (initial) camera position and oreintation 
  if (key == 32)
  {
	  v = vec4(0.0, 1.0, 0.0, 00);
	  u = vec4(1.0, 0.0, 0.0, 0.0);
	  eye = vec4(0.0, 2.0, 3.5, 1.0);
	  n = Angel::normalize(vec4(0.0, 1.0, 2.0, 0.0));
  }

  // positive or negative rotation depending on upper or lower case letter
  if(key > 96)  
  {
    cosine = cos(SCALE_ANGLE * PI/-180.0);
    sine = sin(SCALE_ANGLE * PI/-180.0);
  }
  else
  {
    cosine = cos(SCALE_ANGLE * PI/180.0);
    sine = sin(SCALE_ANGLE * PI/180.0);
  }

  switch(key)
  {
	 
  case 'Z': // roll counterclockwise in the xy plane
  case 'z': // roll clockwise in the xy plane
    xt = u[X];
    yt = u[Y];
    zt = u[Z];
    u[X] = xt*cosine - v[X]*sine;
    u[Y] = yt*cosine - v[Y]*sine;
    u[Z] = zt*cosine - v[Z]*sine;
    v[X] = xt*sine + v[X]*cosine;
    v[Y] = yt*sine + v[Y]*cosine;
    v[Z] = zt*sine + v[Z]*cosine;
    break;
  case 'X'://down
  case 'x': //up
	  xt = v[X];
	  yt = v[Y];
	  zt = v[Z];
	  v[X] = xt*cosine - n[X] * sine;
	  v[Y] = yt*cosine - n[Y] * sine;
	  v[Z] = zt*cosine - n[Z] * sine;
	  n[X] = xt*sine + n[X] * cosine;
	  n[Y] = yt*sine + n[Y] * cosine;
	  n[Z] = zt*sine + n[Z] * cosine;
	  break;
  case 'Y'://side to side
  case 'y':
	  xt = u[X];
	  yt = u[Y];
	  zt = u[Z];
	  u[X] = xt*cosine - n[X] * sine;
	  u[Y] = yt*cosine - n[Y] * sine;
	  u[Z] = zt*cosine - n[Z] * sine;
	  n[X] = xt*sine + n[X] * cosine;
	  n[Y] = yt*sine + n[Y] * cosine;
	  n[Z] = zt*sine + n[Z] * cosine;
	  break;
	case 033:  // Escape key
	case 'q': case 'Q':
	    exit( EXIT_SUCCESS );
    }

  glutPostRedisplay();
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
  default:
    break;
  }
  glutPostRedisplay();
}


void reshape( int width, int height )
//the same objects are shown (possibly scaled) w/o shape distortion 
//original viewport is a square
{

	glViewport( 0, 0, (GLsizei) width, (GLsizei) height );

}

void drawBarrel(){
	
	glUseProgram(program[barrelNum]);

	proj_loc       = glGetUniformLocation(program[barrelNum], "Projection");
	model_view_loc = glGetUniformLocation(program[barrelNum], "ModelView");
	glUniformMatrix4fv(proj_loc, 1, GL_TRUE, projmat);
	glUniformMatrix4fv( model_view_loc, 1, GL_TRUE, model_view);

	glBindVertexArray( vao[0] );
    glDrawArrays( GL_TRIANGLES, 0, CylNumVertices );

	glUseProgram(0);
	glBindVertexArray(0);
	
}

void drawWheel1(){

	glUseProgram(program[wheel1Num]);

	proj_loc       = glGetUniformLocation(program[wheel1Num], "Projection");
	model_view_loc = glGetUniformLocation(program[wheel1Num], "ModelView");
	glUniformMatrix4fv(proj_loc, 1, GL_TRUE, projmat);
	glUniformMatrix4fv( model_view_loc, 1, GL_TRUE, model_view);
	//color_loc = glGetUniformLocation(program[TranslrotRyStretcedCyll], "color");
	//glUniform4fv(color_loc, 1, wheelColor);

	glBindVertexArray(vao[1]);
    glDrawArrays( GL_TRIANGLES, 0, CylNumVertices );

	glUseProgram(0);
}

void drawWheel2() {

	glUseProgram(program[wheel2Num]);

	proj_loc = glGetUniformLocation(program[wheel2Num], "Projection");
	model_view_loc = glGetUniformLocation(program[wheel2Num], "ModelView");
	glUniformMatrix4fv(proj_loc, 1, GL_TRUE, projmat);
	glUniformMatrix4fv(model_view_loc, 1, GL_TRUE, model_view);

	glBindVertexArray(vao[2]);
	glDrawArrays(GL_TRIANGLES, 0, CylNumVertices);

	glUseProgram(0);
}

void drawAxle() {

	glUseProgram(program[axleNum]);

	proj_loc = glGetUniformLocation(program[axleNum], "Projection");
	model_view_loc = glGetUniformLocation(program[axleNum], "ModelView");
	glUniformMatrix4fv(proj_loc, 1, GL_TRUE, projmat);
	glUniformMatrix4fv(model_view_loc, 1, GL_TRUE, model_view);

	glBindVertexArray(vao[3]);
	glDrawArrays(GL_TRIANGLES, 0, CylNumVertices);

	glUseProgram(0);
}