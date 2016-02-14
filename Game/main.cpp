#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <stdlib.h>
#include <unistd.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

void movePlayer();

struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

struct Point
{
	float x, y, z;
};
GLuint programID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

//-----------------------------------GLOBAL OBJECTS------------------------------------------------------



#define PI 3.141592653589
#define ON 1
#define OFF 0
#define DEG2RAD(deg) (float)(deg * PI / 180)
int windowWidth=800, windowHeight=800;
float mouseX=0, mouseY=0, prevMouseX=0, prevMouseY=0;

int adventureView = OFF;
int towerView = ON;
int topView = OFF;
int followcamView = OFF;
int helicopterView=OFF;

int axis=4, fall=OFF, jump=OFF;
float camAngle=90.0f;
int lives=3;
int score=0, gameOver=OFF;
int diffX=0, diffY=0;

Point player = { -5, 0, 5};
Point eyePos = { 8, 3, 6};
Point targetPos = { 0, 0, 0};
Point frontPos = {0, 0, -1};
Point upPos = { 0, 1, 0};
glm::mat4 MVP, VP;

int randVal=1, modVal=7, keyboardCount=0, countSteps=0;

VAO* obstacle();
VAO* Rectangles1();
VAO* Rectangles2();
VAO* Triangles(int up);

//----------------------------------------------------------------------------------------------------------

//------------------------------------KEYBOARD AND MOUSE FUNCTIONS--------------------------------------------
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
     // Function is called first on GLFW_PRESS.
    
	
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_RIGHT:
            	keyboardCount++;
            	if(adventureView==ON)
            	{
            		if(axis==4)
            			axis=1;
            		else if(axis==1)
            			axis=3;
            		else if(axis==3)
            			axis=2;
            		else if(axis==2)
            			axis=4;
            	}
            	else
            	{
            		if((int)(2*(player.x+1)+3*player.z +randVal )% modVal != 0 && player.x+1<5)
            		{	
            			countSteps++;
            			player.x+=1;
            		}
            		else
            		{
            			player.x=-5;
            			player.z=5;
            			score-=5;
            		}
            		axis=4;
            	}

            	break;
            case GLFW_KEY_LEFT:
            	keyboardCount++;
            	if(adventureView==ON)
            	{
            		if(axis==4)
            			axis=2;
            		else if(axis==2)
            			axis=3;
            		else if(axis==3)
            			axis=1;
            		else if(axis==1)
            			axis=4;
            	}
            	else
            	{
            		if((int)(2*(player.x-1)+3*player.z +randVal)% modVal != 0 && player.x-1>=-5)
            		{
            			countSteps++;
            			player.x-=1;
            		}
            		else
            		{
            			player.x=-5;
            			player.z=5;
            			score-=5;
            		}
            		axis=4;
            	}
            	break;
            case GLFW_KEY_UP:
            	keyboardCount++;
            	if(adventureView==ON)
            	{
           			switch(axis)
           			{
           				case 4: 
           					if((int)(2*player.x+3*(player.z-1) +randVal)% modVal != 0 && player.z-1>=-4)
           					{
           						countSteps++;
            					player.z-=1;
           					}
           					else
            				{
            					player.x=-5;
            					player.z=5;
            					score-=5;
            				}
                			break;
                		case 3:
                			if((int)(2*player.x+3*(player.z+1) +randVal)% modVal != 0 && player.z+1<=5)
                			{
                				countSteps++;
            					player.z+=1;
                			}
                			else
            				{
            					player.x=-5;
            					player.z=5;
            					score-=5;
            				}
                			break;
                		case 2:
                			if((int)(2*(player.x-1)+3*(player.z) +randVal)% modVal != 0 && player.x-1>=-5)
                			{
                				countSteps++;
                				player.x-=1;
                			}
                			else
            				{
            					player.x=-5;
            					player.z=5;
            					score-=5;
            				}
                			break;
                		case 1:
                			if((int)(2*(player.x+1)+3*(player.z) +randVal)% modVal != 0 && player.x+1<=4)
                			{
                				countSteps++;
            					player.x+=1;
                			}
                			else
            				{
            					player.x=-5;
            					player.z=5;
            					score-=5;
            				}
                			break;
           			}
           		}
           		else
           		{
           			if((int)(2*player.x+3*(player.z-1) +randVal)% modVal != 0 && player.z-1>=-4)
           			{
           				countSteps++;
            			player.z-=1;
           			}
           			else
            		{
            			player.x=-5;
            			player.z=5;
            			score-=5;
            		}
            		axis=4;
           		}
           		break;
            	
            case GLFW_KEY_DOWN:
            	keyboardCount++;
            	if(adventureView==ON)
            	{
            		switch(axis)
           			{
           				case 4: 
           					if((int)(2*player.x+3*(player.z+1) +randVal)% modVal != 0 && player.z+1<=5)
           					{
           						countSteps++;
            					player.z+=1;
           					}
           					else
            				{
            					player.x=-5;
            					player.z=5;
            					score-=5;
            				}
                			break;
                		case 3:
                			if((int)(2*player.x+3*(player.z-1) +randVal)% modVal != 0 && player.z-1>=-4)
                			{
                				countSteps++;
            					player.z-=1;
                			}
                			else
            				{
            					player.x=-5;
            					player.z=5;
            					score-=5;
            				}
                			break;
                		case 2:
                			if((int)(2*(player.x+1)+3*(player.z) +randVal)% modVal != 0 && player.x+1<=5)
                			{
                				countSteps++;
                				player.x+=1;
                			}
                			else
            				{
            					player.x=-5;
            					player.z=5;
            					score-=5;
            				}
                			break;
                		case 1:
                			if((int)(2*(player.x-1)+3*(player.z) +randVal)% modVal != 0 && player.x-1>=-4)
                			{
                				countSteps++;
            					player.x-=1;
                			}
                			else
            				{
            					player.x=-5;
            					player.z=5;
            					score-=5;
            				}
                			break;
                	}
           		}
           		else
           		{
           			if((int)(2*player.x+3*(player.z+1) +randVal)% modVal != 0 && player.z+1<=5)
            		{
            			countSteps++;
            			player.z+=1;
            		}
            		else
            		{
            			player.x=-5;
            			player.z=5;
            			score-=5;
            		}

            		axis=4;
           		}
           		break;

           	case GLFW_KEY_5:
           		cout<<"helicopterView!\n";
           		helicopterView=ON;
           		towerView=OFF;
            	topView=OFF;
            	adventureView=OFF;
            	followcamView=OFF;
            	eyePos.x=4;
            	eyePos.y=3;
            	eyePos.z=6;
            	break;

            case GLFW_KEY_3:
            	towerView=ON;
            	topView=OFF;
            	adventureView=OFF;
            	followcamView=OFF;
            	helicopterView=OFF;
            	break;
           	case GLFW_KEY_4:
           		towerView=OFF;
            	topView=ON;
            	adventureView=OFF;
            	followcamView=OFF;
            	helicopterView=OFF;
            	break;
            case GLFW_KEY_1:
            	towerView=OFF;
            	topView=OFF;
            	adventureView=ON;
            	followcamView=OFF;
            	helicopterView=OFF;
            	break;
             case GLFW_KEY_2:
            	towerView=OFF;
            	topView=OFF;
            	adventureView=OFF;
            	followcamView=ON;
            	helicopterView=OFF;
            	break;
            case GLFW_KEY_ESCAPE:
                quit(window);
                break;
            case GLFW_KEY_SPACE:
            	jump=ON;
            	break;
            default:
                break;
        }
    }

}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
    switch (key) {
        case 'Q':
        case 'q':
            quit(window);
            break;
        default:
            break;
    }
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            if (action == GLFW_PRESS){
            	cout<<mouseX-prevMouseX<<" "<<mouseY-prevMouseY<<"\n";
            	diffX=mouseX-prevMouseX;
            	diffY=mouseY-prevMouseY;
            	if(mouseY>0)
            		diffY=-1;
            	else
            		diffY=+1;
            	if(mouseX>0)
            		diffX=-1;
            	else
            		diffX=+1;
           	 	upPos.x=0; upPos.y=1; upPos.z=0;
				eyePos.x+=diffX;
				eyePos.z+=diffY;
				eyePos.y=5;
				targetPos.x=eyePos.x;
				targetPos.y=eyePos.y;
				targetPos.z=eyePos.z-10;
				camAngle=90.0f;
            	prevMouseX=mouseX; prevMouseY=mouseY;

            }

            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            if (action == GLFW_RELEASE) {

            }
            break;
        default:
            break;
    }
}
void checkMouseCoordinates(GLFWwindow* window, double x , double y){
    mouseX=(float)x-(float)windowWidth/2;		//Getting pixels into coordinates
    mouseY=(float)windowHeight/2-(float)y;

    //cout<<mouseX<<" "<<mouseY<<"\n";
}
//---------------------------------------------------------------

void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);
    glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);
    //float x=12.0f;
    //Matrices.projection = glm::ortho(-x, x, -x, x, 0.10f, 400.0f);
    Matrices.projection = glm::perspective(camAngle, (GLfloat)fbwidth/(GLfloat)fbheight, 0.1f, 500.0f);

}


GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);
    glfwSetWindowCloseCallback(window, quit);
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks
    glfwSetCursorPosCallback(window, checkMouseCoordinates);
    return window;
}

void initGL (GLFWwindow* window, int width, int height)
{
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");
	reshapeWindow (window, width, height);
	glClearDepth (1.0f);
	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

}
 
VAO* Rectangles1()
{
    static const GLfloat buf [] = {
    0,0,0,     1,0,0,     0,-2,0,
    1,0,0,     1,-2,0,    0,-2,0, 
    };
    static const GLfloat col [] = {
    1,0,0,     1,0,0,     1,0,0, 
    1,0,0,     1,0,0,     1,0,0,     };

    VAO* rectangles = create3DObject(GL_TRIANGLES, 6, buf, col, GL_FILL);
    return rectangles;

}
VAO* Rectangles2()
{
	static const GLfloat buf [] = {
    0,0,0,     1,0,0,     0,-2,0,    1,0,0,
    1,-2,0,
    0,-2,0, 
    };
	static const GLfloat col2 [] = {
    0,0,1,     0,0,1,    0,0,1,
    0,0,1,     0,0,1,     0,0,1,     };

    VAO* rectangles = create3DObject(GL_TRIANGLES, 6, buf, col2, GL_FILL);
    return rectangles;

}

VAO* Triangles(int up)
{
	static const GLfloat vertex_buffer_data [] = {
    0, 0, 0,    0, 1, 0,    1, 0, 0,
    0, 1, 0,    1, 0, 0,    1, 1, 0
  };

  static const GLfloat color_buffer_data [] = {
    1,0,0,     1,1,1,     1,1,1, 
    1,1,1,     1,1,1,     1,0,0,
    };
   static const GLfloat color_buffer_data2 [] = {
    0.35,0.45,0.41,     1,1,1,     1,1,1, 
    1,1,1,     1,1,1,    0.35,0.45,0.41   };
      if(up==0)
  	return create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL); 
  return create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data2, GL_FILL); 


}
VAO* obstacle()
{
	static const GLfloat vertex_buffer_data [] = {
    0, 0, 0,    0.5, 1.5, 0,    1, 0, 0, };

    static const GLfloat color_buffer_data [] = {
    0.2,0.91,1.0,    0.6,0.23,0.56,     0.2,0.91,1.0 };

    return create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_FILL);


}
void drawObstacle(int i, int up, int j)
{
	VAO* O = obstacle();

	Matrices.model = glm::mat4(1.0f);
	Matrices.model*=glm::translate(glm::vec3(i, up, j))*glm::rotate(DEG2RAD(-45), glm::vec3(1,0,0));
	MVP=VP*Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
   	draw3DObject(O);

  	//LEFT side
  	Matrices.model = glm::mat4(1.0f);
	Matrices.model*=glm::translate(glm::vec3(i, up, j))*glm::rotate(DEG2RAD(90), glm::vec3(0,1,0))*glm::rotate(DEG2RAD(45), glm::vec3(1,0,0));
	MVP=VP*Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(O);

	//RIGHT side
	Matrices.model = glm::mat4(1.0f);
	Matrices.model*=glm::translate(glm::vec3(1+i,up,j))*glm::rotate(DEG2RAD(90), glm::vec3(0,1,0))*glm::rotate(DEG2RAD(-45), glm::vec3(1,0,0));
	MVP=VP*Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(O);

	//BACK side
    Matrices.model = glm::mat4(1.0f);
	Matrices.model*=glm::translate(glm::vec3(i,up,-1+j))*glm::rotate(DEG2RAD(45), glm::vec3(1,0,0));
	MVP=VP*Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(O);

	//------------LOWER HALF--------------------------------------------

	j*=-1;
	up*=-1;
	Matrices.model = glm::mat4(1.0f);
	Matrices.model*=glm::rotate(DEG2RAD(180), glm::vec3(1,0,0))*glm::translate(glm::vec3(i, up, j+1))*glm::rotate(DEG2RAD(-45), glm::vec3(1,0,0));
	MVP=VP*Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
   	draw3DObject(O);

  	//LEFT side
  	Matrices.model = glm::mat4(1.0f);
	Matrices.model*=glm::rotate(DEG2RAD(180), glm::vec3(1,0,0))*glm::translate(glm::vec3(i, up, j+1))*glm::rotate(DEG2RAD(90), glm::vec3(0,1,0))*glm::rotate(DEG2RAD(45), glm::vec3(1,0,0));
	MVP=VP*Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(O);

	//RIGHT side
	Matrices.model = glm::mat4(1.0f);
	Matrices.model*=glm::rotate(DEG2RAD(180), glm::vec3(1,0,0))*glm::translate(glm::vec3(1+i,up,j+1))*glm::rotate(DEG2RAD(90), glm::vec3(0,1,0))*glm::rotate(DEG2RAD(-45), glm::vec3(1,0,0));
	MVP=VP*Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(O);

	//BACK side
    Matrices.model = glm::mat4(1.0f);
	Matrices.model*=glm::rotate(DEG2RAD(180), glm::vec3(1,0,0))*glm::translate(glm::vec3(i,up,-1+j+1))*glm::rotate(DEG2RAD(45), glm::vec3(1,0,0));
	MVP=VP*Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(O);

}

void drawAxis()
{
	static const GLfloat X [] = {
		10, 0, 0,		-10, 0, 0,
	};
	static const GLfloat Y [] = {
		0, 10, 0,		0, -10, 0
	};
	static const GLfloat Z [] = {
		0, 0, 10,		0, 0, -10
	};
	static const GLfloat colX [] = {
		1, 1, 1,		1, 1, 1
	};
	static const GLfloat colY [] = {
		0, 1, 0,		0, 1, 0
	};
	static const GLfloat colZ [] = {
		1, 0, 0,		1, 0, 0
	};
	VAO* line1= create3DObject(GL_LINE_STRIP,2, X, colX, GL_FILL);
	VAO* line2= create3DObject(GL_LINE_STRIP,2, Y, colY, GL_FILL);
	VAO* line3= create3DObject(GL_LINE_STRIP,2, Z, colZ, GL_FILL);
	Matrices.model=glm::mat4(1.0f);
	MVP= VP*Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(line1);
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(line2);
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(line3);
}
void createLand()
{
    int up=0;
	VAO* T= Triangles(0);	
	VAO* T2	= Triangles(1);
	VAO* R1= Rectangles1();
	VAO* R2=Rectangles2();
	VAO* oldT=T;

	for(int i=-5; i<5; i++)				//-5 to 5
    {
    	for(int j=-4; j<6; j++)				//-4 to 6
    	{
    		if((2*i+3*j + randVal)% modVal == 0)
    		{
    			up=0;
    			drawObstacle(i, 1, j);
    			oldT=T;
    			T=T2;
    		}
    		else
    		{
    			T=oldT;
    			up=0;
    		}
    		if(player.x==i && player.z==j)
    			up=0;
    		if(i+j==randVal && randVal!=0)
    			continue;

    		//-----------------------------------------------------------------------------------------
    		Matrices.model = glm::mat4(1.0f);
    		Matrices.model*=glm::translate(glm::vec3(i, up, j));
    		MVP=VP*Matrices.model;
        	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
       		draw3DObject(R1);

  			//LEFT side
  		  	Matrices.model = glm::mat4(1.0f);
        	Matrices.model*=glm::translate(glm::vec3(i, up, j))*glm::rotate(DEG2RAD(90), glm::vec3(0,1,0));
        	MVP=VP*Matrices.model;
        	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        	draw3DObject(R2);

    		//RIGHT side
    		Matrices.model = glm::mat4(1.0f);
        	Matrices.model*=glm::translate(glm::vec3(1+i,up,j))*glm::rotate(DEG2RAD(90), glm::vec3(0,1,0));
        	MVP=VP*Matrices.model;
        	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        	draw3DObject(R2);

    		//BACK side
    		Matrices.model = glm::mat4(1.0f);
        	Matrices.model*=glm::translate(glm::vec3(i,up,-1+j));
        	MVP=VP*Matrices.model;
        	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        	draw3DObject(R1);
    		//-----------------------------------------------------------------------------------------
		  	
		  	Matrices.model = glm::mat4(1.0f);
		  	Matrices.model*=glm::translate(glm::vec3(i, up, j))*glm::rotate(DEG2RAD(-90), glm::vec3(1, 0,0));
		  	MVP=VP*Matrices.model;
		  	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
		 	draw3DObject(T);
    	}
    }

    if(keyboardCount>6)			//after 2 consecutive press and releases
	{	randVal= rand() % 10;	keyboardCount=0; }
       
}
void movePlayer()
{
 VAO *rectangle1, *rectangle2, *triangle;

    static const GLfloat buf [] = {
    0,0,0,     1,0,0,     0,1,0,     1,0,0,
    1,1,0,    0,1,0,     };

    static const GLfloat col [] = {
    0,0.39,0,     0,0.39,0,    0,0.39, 0,
    0,0.39,0,    0,0.39,0,     0,0.39, 0,    };

    static const GLfloat col2 [] = {
    0,1,0,     0,1,0,     0,1,0, 
    0,1,0,     0,1,0,     0,1,0,     };

    static const GLfloat vertex_buffer_data [] = {
    0, 0, 0,    0, 1, 0,    1, 0, 0,
    0, 1, 0,    1, 0, 0,    1, 1, 0
	};

  static const GLfloat color_buffer_data [] = {
    0,1,0,     0,0,0,     0,1,0, 
    0,0,0,     0,1,0,     0,1,0, 	};

	triangle=create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
    rectangle1 = create3DObject(GL_TRIANGLES, 6, buf, col, GL_FILL);
    rectangle2= create3DObject(GL_TRIANGLES, 6, buf, col2, GL_FILL);
    //FRONT side
    Matrices.model = glm::mat4(1.0f);
    	Matrices.model*=glm::translate(glm::vec3(player.x, player.y, player.z));
    	MVP=VP*Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(rectangle1);

    //LEFT side
    Matrices.model = glm::mat4(1.0f);
        Matrices.model*=glm::translate(glm::vec3(player.x, player.y, player.z))*glm::rotate(DEG2RAD(90), glm::vec3(0,1,0));
        MVP=VP*Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(rectangle2);

    //RIGHT side
    Matrices.model = glm::mat4(1.0f);
        Matrices.model*=glm::translate(glm::vec3(player.x+1, player.y, player.z))*glm::rotate(DEG2RAD(90), glm::vec3(0,1,0));
        MVP=VP*Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(rectangle2);

    //BACK side
    Matrices.model = glm::mat4(1.0f);
        Matrices.model*=glm::translate(glm::vec3(player.x, player.y, player.z-1));
        MVP=VP*Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(rectangle1);

    //Top
    Matrices.model = glm::mat4(1.0f);
    	Matrices.model*=Matrices.model*glm::translate(glm::vec3(player.x, player.y+1, player.z))*glm::rotate(DEG2RAD(-90), glm::vec3(1, 0,0));
		MVP=VP*Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
		draw3DObject(triangle);

}
void getLookAtAttributes()
{
	if(towerView == ON)
	{
		eyePos.x=6; eyePos.y=3; eyePos.z=6;
		targetPos.x=targetPos.y=targetPos.z=0;
		upPos.x=0; upPos.y=1; upPos.z=0;
		camAngle=90.0f;
	}
	else if ( topView == ON)
	{
		eyePos.x=0; eyePos.y=7; eyePos.z=0;
		targetPos.x=targetPos.y=targetPos.z=0;
		upPos.y=0; upPos.z=-1; upPos.x=0;
		camAngle=90.0f;
	}
	else if(adventureView == ON)
	{
		upPos.x=0; upPos.y=1; upPos.z=0;
		if(axis==4)
		{
			eyePos.x=player.x+0.5; eyePos.y=player.y+1; eyePos.z=player.z-1;
			targetPos.x=player.x+0.5; targetPos.y=player.y-1; targetPos.z=player.z-5;
			camAngle=90.0f;
		}
		else if(axis==3)
		{
			eyePos.x=player.x+0.5; eyePos.y=player.y+1; eyePos.z=player.z+1;
			targetPos.x=player.x+0.5; targetPos.y=player.y-1; targetPos.z=player.z+5;
			camAngle=90.0f;
		}
		else if(axis==2)
		{
			eyePos.x=player.x-0.5; eyePos.y=player.y+1; eyePos.z=player.z-0.5;
			targetPos.x=player.x-5; targetPos.y=player.y-1; targetPos.z=player.z-0.5;
			camAngle=90.0f;
		}
		else if(axis==1)
		{
			eyePos.x=player.x+1; eyePos.y=player.y+1; eyePos.z=player.z-0.5;
			targetPos.x=player.x+5; targetPos.y=player.y-1; targetPos.z=player.z-0.5;
			camAngle=90.0f;
		}
	}
	else if (followcamView == ON)
	{
		upPos.x=0; upPos.y=1; upPos.z=0;
		eyePos.x=player.x+1; eyePos.y=player.y+3; eyePos.z=player.z+1;
		targetPos.x=player.x+1; targetPos.y=player.y; targetPos.z=player.z-5;
		camAngle=90.0f;

	}
	else if(helicopterView == ON)
	{
		/*upPos.x=0; upPos.y=1; upPos.z=0;
		eyePos.x+=diffX;
		eyePos.y+=diffY;
		eyePos.z=5;
		targetPos.x=eyePos.x;
		targetPos.y=eyePos.y;
		targetPos.z=eyePos.z-10;
		camAngle=90.0f;*/

	}
}
void  checkIfFalling()
{
	if(player.x+player.z==randVal || player.x>6 || player.x<-5 || player.z>5 || player.z<-4)
 	{
     	fall=ON;
 		while(player.y>=-10)
 		{ player.y-=1; movePlayer(); usleep(1); }
 		
 		lives--;
 		fall=OFF;
 		player.x=-5; player.y=0; player.z=5;
 	}
}
void Jump()
{
	if(adventureView==ON)
	{
		switch(axis)
		{
			case 4: 
				player.y+=1;
 				movePlayer();
 				player.z-=2;
			 	movePlayer();
			 	player.y-=1;
			 	movePlayer();
			 	break;
			case 3:
				player.y+=1;
 				movePlayer();
 				player.z+=2;
			 	movePlayer();
			 	player.y-=1;
			 	movePlayer();
			 	break;
			case 2:
				player.y+=1;
 				movePlayer();
 				player.x-=2;
			 	movePlayer();
			 	player.y-=1;
			 	movePlayer();
			 	break;
			case 1:
				player.y+=1;
 				movePlayer();
 				player.x+=2;
			 	movePlayer();
			 	player.y-=1;
			 	movePlayer();
			 	break;
			default: break;
		}
	}
	else
	{
		player.y+=1;
 		movePlayer();
 		player.z-=2;
 		movePlayer();
 		player.y-=1;
 		movePlayer();
 	}

 	jump=OFF;
}
void draw ()
{
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram (programID);

	getLookAtAttributes();
	
	glm::vec3 eye(eyePos.x, eyePos.y, eyePos.z);
	glm::vec3 target(targetPos.x, targetPos.y, targetPos.z);
	glm::vec3 up (upPos.x, upPos.y, upPos.z);
	Matrices.view = glm::lookAt(eye, target, up);

	VP= Matrices.projection*Matrices.view;
	//drawAxis();
	createLand();
 	movePlayer();

 	if(jump==ON)
 	{
 		Jump();
 		jump=OFF;
 	}

 	checkIfFalling();
 	if(player.x==4 && player.z==-4)
 	{
 		score+=50;
 		cout<<"You win! Score: "<<score<<"\n ";
 		gameOver=ON;
 	}
 	else if(lives==0)
 	{
 		score-=50;
 		cout<<"Sorry! You lost! Score: "<<score<<"\n";
 		gameOver=ON;
 	}
 	
}

int main (int argc, char** argv)
{

    GLFWwindow* window = initGLFW(windowWidth, windowHeight);

	initGL (window, windowWidth, windowHeight);

    double last_update_time = glfwGetTime(), current_time;

    while (!glfwWindowShouldClose(window)) {
        draw();
        glfwSwapBuffers(window);
        glfwPollEvents();
        current_time = glfwGetTime(); // Time in seconds
        if ((current_time - last_update_time) >= 0.5) { // atleast 0.5s elapsed since last frame
            last_update_time = current_time;
        }
        if(gameOver==ON)
        	quit(window);
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}