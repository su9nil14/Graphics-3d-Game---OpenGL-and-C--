//Some Windows Headers (For Time, IO, etc.)
#include <windows.h>
#include <time.h>
#include <mmsystem.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include "maths_funcs.h" 
#include "VCamera.h"
#include "VboObject.h" 
#include "text.h"
#include <math.h>
#include <include/irrKlang.h>

using namespace std;
using namespace irrklang;

#pragma comment(lib, "irrKlang.lib")

#define SPACEBAR 32
#define MOUSE_SENSITIVITY 0.005f

// this mesh is a dae file format but you should be able to use any other format too, obj is typically what is used
// put the mesh in your project directory, or provide a filepath for it here
#define MESH_NAME1 "../Media/environment2.dae"
//#define MESH_NAME1 "../Media/ground.dae"
#define MESH_NAME2 "../Media/cube.obj"
#define MESH_NAME3 "../Media/monkey.obj"
#define MESH_NAME4 "../Media/bullet.dae"


#define TEXTURE_NAME1 "../Media/environment.png"
//#define TEXTURE_NAME1 "../Media/ground.png"
#define TEXTURE_NAME2 "../Media/cube.png"
#define TEXTURE_NAME3 "../Media/monkey.png"
#define TEXTURE_NAME4 "../Media/bullet.png"


// files to use for font. change path here
const char* atlas_image = "../Media/freemono.png";
const char* atlas_meta = "../Media/freemono.meta";

// text id for timer (each string of on-screen text has an id number in case you want to update it later)
char tmp[256];
int win_id = 0;
int health_id = 0;
int start_id = 0;
int time_id = -1;
/*----------------------------------------------------------------------------
----------------------------------------------------------------------------*/
static VboObject ground ;
static VboObject cube ;
static VboObject monkey ;
static VboObject bullet ;

GLuint Texture1; //groung
GLuint Texture2; //cube
GLuint Texture3; //monkey
GLuint Texture4; //bullet

GLuint shaderProgramID;
GLuint text_vao;

static VCamera camera;

//bool wireframe=false;
int width = 800;
int height = 600;
int windowId;


float delta;

GLfloat rotate_y = 0.0f;

boolean keyPressed = false ;
boolean startGame = false;
float rand_x, rand_y, rand_z;

// input variables
float fov = 67.0f * ONE_DEG_IN_RAD; // convert 67 degrees to radians
float aspect = (float)width / (float)height; // aspect ratio

mat4 model_cube;
// bullets 
std::vector<mat4> model_monkey;
boolean bulletdraw = false;
std::vector<mat4> bullets;

int killcount = 0;
int health = 50;
bool lose = false;
bool win = false;

ISoundEngine* se;

// Shader Functions- click on + to expand===================================================================================================
#pragma region SHADER_FUNCTIONS

// Create a NULL-terminated string by reading the provided file
char* readShaderSource(const char* shaderFile) {   
	FILE* fp = fopen(shaderFile, "rb"); //!->Why does binary flag "RB" work and not "R"... wierd msvc thing?

	if ( fp == NULL ) { return NULL; }

	fseek(fp, 0L, SEEK_END);
	long size = ftell(fp);

	fseek(fp, 0L, SEEK_SET);
	char* buf = new char[size + 1];
	fread(buf, 1, size, fp);
	buf[size] = '\0';

	fclose(fp);

	return buf;
}


static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	// create a shader object
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0) {
		fprintf(stderr, "Error creating shader type %d\n", ShaderType);
		exit(0);
	}
	const char* pShaderSource = readShaderSource( pShaderText);

	// Bind the source code to the shader, this happens before compilation
	glShaderSource(ShaderObj, 1, (const GLchar**)&pShaderSource, NULL);
	// compile the shader and check for errors
	glCompileShader(ShaderObj);
	GLint success;
	// check for shader related errors using glGetShaderiv
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024];
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
		exit(1);
	}
	// Attach the compiled shader object to the program object
	glAttachShader(ShaderProgram, ShaderObj);
}

GLuint CompileShaders()
{
	//Start the process of setting up our shaders by creating a program ID
	//Note: we will link all the shaders together into this ID
	shaderProgramID = glCreateProgram();
	if (shaderProgramID == 0) {
		fprintf(stderr, "Error creating shader program\n");
		exit(1);
	}

	// Create two shader objects, one for the vertex, and one for the fragment shader
	AddShader(shaderProgramID, "../Shaders/simpleVertexShader.txt", GL_VERTEX_SHADER);
	AddShader(shaderProgramID, "../Shaders/simpleFragmentShader.txt", GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { 0 };
	// After compiling all shader objects and attaching them to the program, we can finally link it
	glLinkProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
		exit(1);
	}

	// program has been successfully linked but needs to be validated to check whether the program can execute given the current pipeline state
	glValidateProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
		exit(1);
	}

	// Finally, use the linked shader program
	// Note: this program will stay in effect for all draw calls until you replace it with another or explicitly disable its use
	glUseProgram(shaderProgramID);
	return shaderProgramID;
}

/*============================================================================================================================================*/

/*DISPLAY function=================================================================================================================*/

////collison detection
boolean collisionDetection(vector<mat4> test1, vector<mat4> test2) {

	bool collision = false;

	for(int i =0; i < test1.size();i++)
	{

		for(int j =0; j < test2.size();j++){

			vec3 from = vec3(test1[i].m[12],test1[i].m[13],test1[i].m[14]);
			vec3 to = vec3(test2[j].m[12],test2[j].m[13],test2[j].m[14]);
			if(get_squared_dist(from,to)<3  ){

				collision = true;
			}
		}
	}
	return collision;
}

//reload the monkeys
void init_games(){

		model_monkey.push_back(translate (identity_mat4(), vec3(rand_x, 12, -rand_x)));
}

//update bullets i.e move them + delete them if it hits target
void shoot(){


	for(int i =0; i < bullets.size();i++)
	{
		bullets[i].m[12] += camera.cameraFront.x *delta*40;
		bullets[i].m[13] += camera.cameraFront.y *delta*40;
		bullets[i].m[14] += camera.cameraFront.z *delta*40;

			//if bullet goes off ground boundary delete them..
			if(bullets[i].m[12]> 99.0f ||bullets[i].m[14] < -99.0f  ){
				bullets.clear();
			}

		for(int j =0; j < model_monkey.size();j++)
		{

			if(collisionDetection(bullets,model_monkey) ) {
				model_monkey.clear();
				bullets.clear();
				killcount++;
				if(killcount >  health ) 
					win =true;
			}

			//if all monkeys killed make more
			if(model_monkey.size()<=0  )
			{
				init_games();
			}
		}//for loop j

	}//for loop i
}


void display(){

	// tell GL to only draw onto a pixel if the shape is closer to the viewer
	glEnable (GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc (GL_LESS); // depth-testing interprets a smaller value as "closer"
	glClearColor (0.3f, 0.3f, 1.0f, 1.0f);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram (shaderProgramID);


	//Declare your uniform variables that will be used in your shader
	int matrix_location = glGetUniformLocation (shaderProgramID, "model");
	int view_mat_location = glGetUniformLocation (shaderProgramID, "view");
	int proj_mat_location = glGetUniformLocation (shaderProgramID, "proj");
	//int light_location = glGetUniformLocation (shaderProgramID, "lightpos");

	glm::mat4 persp_proj = glm::perspective(fov, aspect, 0.1f, 200.0f);
	glm::mat4 model = glm::mat4(1.0f);
	vec3 cam_orient = vec3(0.0f, 0.0f, 0.0f);
	glm::mat4 view = camera.getViewMatrix();

	//===========================================================================================
	//--ground
	//model = glm::translate(model, glm::vec3(-10.0f, -10.0f, 0.0f));
	// update uniforms & draw 
	glBindTexture(GL_TEXTURE_2D,Texture1);
	glUniformMatrix4fv (proj_mat_location, 1, GL_FALSE,  glm::value_ptr(persp_proj)); 
	glUniformMatrix4fv (view_mat_location, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv (matrix_location, 1, GL_FALSE, glm::value_ptr(model));
	ground.draw();

	//===========================================================================================

	//model2 --cube
	glm::mat4 model_cube = glm::mat4(1.0f);
	model_cube = glm::translate(model_cube, glm::vec3(61.0f, 12.0f, -63.0f));

	// update uniform & draw
	glBindTexture(GL_TEXTURE_2D,Texture2);
	glUniformMatrix4fv (matrix_location, 1, GL_FALSE, glm::value_ptr(model_cube));	
	cube.draw();

	//--cube- 2
	model_cube = glm::rotate(model_cube, rotate_y, glm::vec3(0.0f, 1.0f, 0.0f));
	model_cube = glm::translate(model_cube, glm::vec3(-2.0f, 3.0f, -0.0f));

	// update uniform & draw
	glBindTexture(GL_TEXTURE_2D,Texture2);
	glUniformMatrix4fv (matrix_location, 1, GL_FALSE, glm::value_ptr(model_cube));	
	cube.draw();
	//===========================================================================================

	if(bulletdraw) {
		shoot();

		for(int i = 0; i < bullets.size(); i++) {

			glBindTexture(GL_TEXTURE_2D, Texture4);
			glUniformMatrix4fv (matrix_location, 1, GL_FALSE, bullets[i].m);
			bullet.draw();
		}
	}

	for(int i=0; i<model_monkey.size(); i++){
		// update uniform & draw
		glBindTexture(GL_TEXTURE_2D,Texture3);
		glUniformMatrix4fv (matrix_location, 1, GL_FALSE, model_monkey[i].m);	
		monkey.draw();
	}
	//===========================================================================================

	// draw all the texts
	draw_texts(); 

	glutSwapBuffers();
}

/*=======================================================================================================================================*/


void updateScene() {	

	// Placeholder code, if you want to work with framerate
	// Wait until at least 16ms passed since start of last frame (Effectively caps framerate at ~60fps)
	static DWORD  last_time = 0;
	DWORD  curr_time = timeGetTime();
	delta = (curr_time - last_time);

	if (delta > 0.03f)
		delta = 0.03f;
	last_time = curr_time;
	// rotate the model slowly around the y axis
	rotate_y+=0.004f;

	// update the text for the timer
	double t = (double)glutGet(GLUT_ELAPSED_TIME) / 1000.0;
	float r = fabs(sinf(t));
	float g = fabs(sinf(t + 1.57f));

	if(win){
		sprintf(tmp, "You win with %d kills", killcount);
		update_text(start_id, tmp);
		change_text_colour(time_id, 0.0f, g, 0.0f, 1.0f);
		health =50;
	}

	if(health < 0 && !win){
		sprintf(tmp, "You lose", 0);
		update_text(start_id, tmp);
		change_text_colour(time_id, 1.0f, 0.0f, 0.0f, 1.0f);
		bullets.clear();
		model_monkey.clear();

	}
	if(health >= 0 && !win){
	sprintf(tmp, "Total kills: %d Health: %d", killcount, health);
	update_text(time_id, tmp);
	change_text_colour(time_id, 0.0f, g, 0.0f, 1.0f);
	}
	// Draw the nextframe
	static int x=0;
	srand(time(NULL)+x);//init random
	x+=10;
	rand_x = (rand() % 90);



	glutPostRedisplay();
}


void init(){


	std::cout<<"Initializing scene..."<<std::endl;
	

	//setup camera
	camera = VCamera();
	camera.position =glm::vec3(3.0f, 10.0f, -4.0f);
	camera.speed = 10.0f;

	// Set up the shaders
	shaderProgramID = CompileShaders();

	// Load the texture ******************************************************************
	Texture1 = ground.loadTexture(TEXTURE_NAME1);
	Texture2 = cube.loadTexture(TEXTURE_NAME2);
	Texture3 = monkey.loadTexture(TEXTURE_NAME3);
	Texture4 = bullet.loadTexture(TEXTURE_NAME4);

	// load mesh into a vertex buffer array
	ground.myObject(MESH_NAME1, shaderProgramID);
	cube.myObject(MESH_NAME2, shaderProgramID) ;
	monkey.myObject(MESH_NAME3, shaderProgramID);
	bullet.myObject(MESH_NAME4, shaderProgramID);


	//model_monkey.push_back(translate (identity_mat4(), vec3(55, 15, -63)));	


	// Texts
	// initialise font, load from files
	if (!init_text_rendering(atlas_image, atlas_meta, width, height)) {
		fprintf(stderr, "ERROR init text rendering\n");
		return ;
	}

	start_id = add_text (
		"Press g to start the game\n And Shoot the monkeys with spacebar\n  PRESS 0 TO QUIT",
		-1.0f, -0.5f, 35.0f, 0.5f, 0.5f, 1.0f, 1.0f);

	time_id = add_text (
		"Time is: %f",
		-1.0f, 1.0f, 40.0f, 1.0f, 0.0f, 0.0f, 1.0f);

}


/*Required for mouse and keyboard fuctions==============================================================================================*/
bool positionInWindow( double mouseX, double mouseY) {

	return (mouseX > 0 && mouseX < width) && (mouseY > 0 && mouseY < height);
}

void windowReshapeFunc( GLint newWidth, GLint newHeight ) {
	glViewport( 0, 0, newWidth, newHeight );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluOrtho2D( 0, GLdouble (newWidth), 0, GLdouble (newHeight) );
	//gluPerspective(fov, aspect, 0.1f, 200.0f);


	width = newWidth;
	height = newHeight;

}

void handleCursorPosition(int xpos, int ypos) {
	static double last_xpos = xpos;
	static double last_ypos = ypos;

	if (startGame  && positionInWindow(xpos, ypos)) {
		double xpos_delta;
		double ypos_delta;

		xpos_delta = (last_xpos - xpos) * MOUSE_SENSITIVITY;
		ypos_delta = (last_ypos - ypos) * MOUSE_SENSITIVITY;

		camera.mouseUpdate(xpos_delta, ypos_delta);

	}

	last_xpos = xpos;
	last_ypos = ypos;
}


// Placeholder code for the keypress
void keypress(unsigned char key, int x, int y) {

	// Camera controls
	if(key == 'g'){
		init_games();
		sprintf(tmp, "Shoot the monkeys", 0);
		update_text(start_id, tmp);
		startGame = true;
	}

	if (key=='w'){
		camera.moveForward(delta);
		keyPressed = true;
	}
	if (key=='s'){
		camera.moveBackward(delta);
		keyPressed = true;
	}
	if (key=='a'){
		camera.moveLeft(delta);
		keyPressed = true;
	}
	if (key=='d'){
		camera.moveRight(delta);
		keyPressed = true;
	}
	
	//for every bullet fired -1 health.. so make it count :P
	if(key == SPACEBAR){
		se->play2D("../Audio/shoot.wav", false);
		bullets.push_back(scale (identity_mat4(), vec3 (0.05,0.05,0.05)));
		bullets.push_back(translate (identity_mat4(), vec3 (camera.position.x, camera.position.y, camera.position.z)));
		bulletdraw = true;	
		health--;
	}
	
	if(key == '0'){//quit
		keyPressed = false;
		glutDestroyWindow(windowId);
	}



	glutPostRedisplay();
}


/*MAIN=================================================================================================================================*/
int main(int argc, char** argv){

	//sound
	se = createIrrKlangDevice();
	if(!se){
		cout << "Error: Sound Engine could not be created" << endl;
	}

	// Set up the window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB| GLUT_DEPTH);
	glutInitWindowSize(width, height);
	windowId = glutCreateWindow("Lab 6 Game");
	// Tell glut where the display function is
	glutDisplayFunc(display);
	glutIdleFunc(updateScene);
	glutKeyboardFunc(keypress);
	glutSetCursor(GLUT_CURSOR_NONE); 
	glutPassiveMotionFunc(handleCursorPosition);
	glutReshapeFunc(windowReshapeFunc);


	// A call to glewInit() must be done after glut is initialized!
	GLenum res = glewInit();
	// Check for any errors
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}
	// Set up your objects and shaders
	init();
	// Begin infinite event loop
	glutMainLoop();

	se->drop(); // delete engine
	return 0;

}











