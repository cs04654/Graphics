//********************************
//Αυτό το αρχείο θα το χρησιμοποιήσετε
// για να υλοποιήσετε την άσκηση 1Γ της OpenGL
//
//ΑΜ: 4739   Όνομα: ΜΠΑΛΛΟΣ  ΕΥΑΓΓΕΛΟΣ
//ΑΜ: 4654   Όνομα: ΓΚΙΟΥΛΗΣ ΚΩΝΣΤΑΝΤΙΝΟΣ

//*********************************
#define _CRT_SECURE_NO_WARNINGS

#define STB_IMAGE_IMPLEMENTATION

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <iostream>
//#include "/usr/home/students/stud19/cse94654/assignment-1c/irrKlang-64bit-1.6.0/include/irrKlang.h"
#include <fstream>
#include <algorithm>
#include <sstream>
#include "stb_image.h"
#include "gridarray.h"
//#include "Winmm.lib"

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;
using namespace std;
//using namespace irrklang;

#include "controls.hpp"
#include "controls.cpp"

#define STEP 0.25
GLuint Textures_array[2];



//glm::mat4 ViewMatrix;
//glm::mat4 ProjectionMatrix;



//*******************************************************************************
// Η παρακάτω συνάρτηση είναι από http://www.opengl-tutorial.org/beginners-tutorials/tutorial-7-model-loading/
// H συνάρτηση loadOBJ φορτώνει ένα αντικείμενο από το obj αρχείο του και φορτώνει και normals kai uv συντεταγμένες
// Την χρησιμοποιείτε όπως το παράδειγμα που έχω στην main
// Very, VERY simple OBJ loader.
// 
bool loadOBJ(
	const char* path, 
	std::vector<glm::vec3>& out_vertices,
	std::vector<glm::vec2>& out_uvs,
	std::vector<glm::vec3>& out_normals) 
{
	printf("Loading OBJ file %s...\n", path);

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;


	FILE* file = fopen(path, "r");
	if (file == NULL) {
		printf("Impossible to open the file ! Are you in the right path ? See Tutorial 1 for details\n");
		getchar();
		return false;
	}

	while (1) {

		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

		// else : parse lineHeader

		if (strcmp(lineHeader, "v") == 0) {
			glm::vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			temp_vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			glm::vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			uv.y = -uv.y; // Invert V coordinate since we will only use DDS texture, which are inverted. Remove if you want to use TGA or BMP loaders.
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			glm::vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9) {
				printf("File can't be read by our simple parser :-( Try exporting with other options\n");
				fclose(file);
				return false;
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}
		else {
			// Probably a comment, eat up the rest of the line
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}

	}

	// For each vertex of each triangle
	for (unsigned int i = 0; i < vertexIndices.size(); i++) {

		// Get the indices of its attributes
		unsigned int vertexIndex = vertexIndices[i];
		unsigned int uvIndex = uvIndices[i];
		unsigned int normalIndex = normalIndices[i];

		// Get the attributes thanks to the index
		glm::vec3 vertex = temp_vertices[vertexIndex - 1];
		glm::vec2 uv = temp_uvs[uvIndex - 1];
		glm::vec3 normal = temp_normals[normalIndex - 1];

		// Put the attributes in buffers
		out_vertices.push_back(vertex);
		out_uvs.push_back(uv);
		out_normals.push_back(normal);

	}
	fclose(file);
	return true;
}


// https://www.khronos.org/opengl/wiki/Texture#Render_targets
GLuint loadTexture(const char* path,int width,int height ,int n, int tOffset)
{
	glGenTextures(1,&Textures_array[tOffset]);
	glBindTexture(GL_TEXTURE_2D,Textures_array[tOffset]);
	unsigned char *data = stbi_load(path, &width, &height, &n, 0);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,width,height,0,GL_RGB,GL_UNSIGNED_BYTE,data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	return Textures_array[tOffset];

}

//we use system command to play sound 
//opengl is not supposed to play sound 
//we could use openAL but we need to download it
void playSound(){
	system("aplay 'Explosion (sound effect) (128 kbps).mp3'");
}





//************************************
// Η LoadShaders είναι black box για σας
//************************************
GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if (VertexShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << VertexShaderStream.rdbuf();
		VertexShaderCode = sstr.str();
		VertexShaderStream.close();
	}
	else {
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
		getchar();
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if (FragmentShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << FragmentShaderStream.rdbuf();
		FragmentShaderCode = sstr.str();
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;


	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const* VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}



	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const* FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}



	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}


	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}


///****************************************************************
//  Εδω θα υλοποιήσετε την συνάρτηση της κάμερας
//****************************************************************


glm::mat4 camera_function(glm::vec3* coords, float* fov)
{
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) // FORWARD
			if (coords->x <  100) // After this, object disappears
				coords->x += STEP;
	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) // BACKWARD
		if (coords->x > -100) // After this, object disappears
			coords->x -= STEP;

	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) // UPWARD
		if (coords->z <  100) // After this, object disappears
			coords->z += STEP;
	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) // DOWNWARD
		if (coords->z > -100) // After this, object disappears
			coords->z -= STEP;

	// E and C buttons are for keyboards without a numpad. Long live 60%
	if (glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) // ZOOM IN
		if (*fov > 5) // Prevents camera mirroring
			*fov -= 2*STEP;
	if (glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) // ZOOM OUT
		if (*fov < 150) // Prevents zooming out too much
			*fov += 2*STEP;

	// Camera things
	glm::mat4 Projection = glm::perspective(glm::radians(*fov), 4.0f / 4.0f, 0.1f, 100.0f);
	glm::mat4 View = glm::lookAt(
        *coords, 
		glm::vec3(coords->x + 0.1f, coords->y + 0.1f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f)
    );

    // Model matrix : an identity matrix (model will be at the origin)
	glm::mat4 Model = glm::mat4(1.0f);
	// Our ModelViewProjection : multiplication of our 3 matrices
	return Projection * View * Model; // Remember, matrix multiplication is the other way around
}

int main(void)
{
	glEnable ( GL_LIGHTING ) ; // enabling lighting changed
	glEnable ( GL_COLOR_MATERIAL ) ; // enabling color changed
	glGenTextures(2,Textures_array);
	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(1000, 1000, "Εργασία 1Γ - Καταστροφή", NULL, NULL);


	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// Black background
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
		
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	
	//GLuint programID = LoadShaders("ProjCVertexShader.vertexshader", "ProjCFragmentShader.fragmentshader");
	//"StandardShading.vertexshader","StandardShading.fragmentshader"
	GLuint programID = LoadShaders("StandardShading.vertexshader","StandardShading.fragmentshader");
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	GLuint ModelMatrixID = glGetUniformLocation(programID, "M");
	GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
	GLuint MVP = glGetUniformLocation(programID, "MV");
	GLuint power2 = glGetUniformLocation(programID, "power");

	

	// Initial camera position
	//glm::vec3 coords = glm::vec3(10.0f, 10.0f, 20.0f);

	srand(time(NULL));
	float fov = 45.0f;

	// Draw plane
	//int* plane = (int*) malloc(2*3*20*20*sizeof(float));
	GLfloat g_vertex_buffer_data[3*3*2*20*20];
	//GLfloat g_uv_buffer_data[3*3*2*20*20];
	for (int x = 0; x < 19; ++x)
	{
		for (int y = 0; y < 19; ++y)
		{
			// 0,0 0,1 0,2 1,0 1,1 1,2
			int offset = (x*19 + y)*18;

			// first triangle of square
			g_vertex_buffer_data[offset + 0]  = (float) x; //x
			g_vertex_buffer_data[offset + 1]  = (float) y; //y
			g_vertex_buffer_data[offset + 2]  = 0.0f; //z

			g_vertex_buffer_data[offset + 3]  = (float) x + 1;
			g_vertex_buffer_data[offset + 4]  = (float) y;
			g_vertex_buffer_data[offset + 5]  = 0.0f;

			g_vertex_buffer_data[offset + 6]  = (float) x + 1;
			g_vertex_buffer_data[offset + 7]  = (float) y + 1;
			g_vertex_buffer_data[offset + 8]  = 0.0f;

			// second triangle of square
			g_vertex_buffer_data[offset + 9]  = (float) x;
			g_vertex_buffer_data[offset + 10] = (float) y;
			g_vertex_buffer_data[offset + 11] = .0f;

			g_vertex_buffer_data[offset + 12] = (float) x;
			g_vertex_buffer_data[offset + 13] = (float) y + 1;
			g_vertex_buffer_data[offset + 14] = 0.0f;

			g_vertex_buffer_data[offset + 15] = (float) x + 1;
			g_vertex_buffer_data[offset + 16] = (float) y + 1;
			g_vertex_buffer_data[offset + 17] = 0.0f;

			/*for (int i = 0; i < 18; ++i)
			{
				g_uv_buffer_data[offset + i] = 0.8f;
			}*/

		}
		
	}

	GLfloat normals_data[3*3*2*20*20];
	//GLfloat g_uv_buffer_data[3*3*2*20*20];
	for (int x = 0; x < 19; ++x)
	{
		for (int y = 0; y < 19; ++y)
		{
			// 0,0 0,1 0,2 1,0 1,1 1,2
			int offset = (x*19 + y)*18;

			// first triangle of square
			g_vertex_buffer_data[offset + 0]  = (float) x; //x
			g_vertex_buffer_data[offset + 1]  = (float) y; //y
			g_vertex_buffer_data[offset + 2]  = 0.0f; //z

			g_vertex_buffer_data[offset + 3]  = (float) x + 1;
			g_vertex_buffer_data[offset + 4]  = (float) y;
			g_vertex_buffer_data[offset + 5]  = 0.0f;

			g_vertex_buffer_data[offset + 6]  = (float) x + 1;
			g_vertex_buffer_data[offset + 7]  = (float) y + 1;
			g_vertex_buffer_data[offset + 8]  = 0.0f;

			// second triangle of square
			g_vertex_buffer_data[offset + 9]  = (float) x;
			g_vertex_buffer_data[offset + 10] = (float) y;
			g_vertex_buffer_data[offset + 11] = .0f;

			g_vertex_buffer_data[offset + 12] = (float) x;
			g_vertex_buffer_data[offset + 13] = (float) y + 1;
			g_vertex_buffer_data[offset + 14] = 0.0f;

			g_vertex_buffer_data[offset + 15] = (float) x + 1;
			g_vertex_buffer_data[offset + 16] = (float) y + 1;
			g_vertex_buffer_data[offset + 17] = 0.0f;

			/*for (int i = 0; i < 18; ++i)
			{
				g_uv_buffer_data[offset + i] = 0.8f;
			}*/

		}
		
	}	

	/*for (int x = 0; x < 20; ++x)
	{
		for (int y = 0; y < 20; ++y)
		{
			for (int i = 0; i < 18; ++i)
			{
				printf("%f ", g_vertex_buffer_data[(x*20 + y)*18 + i]);
			}
			printf("\n");
		}
		printf("\n");
	}*/
	

	loadTexture("textures/ground2.jpg",715,715,4,0);

	GLuint textures_array_id[2];
	textures_array_id[0] = glGetUniformLocation(programID, "myTextureSampler");

	// Read our .obj file
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> uvs;
	bool res = loadOBJ("Obj files/ball.obj", vertices, uvs, normals);

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	GLuint normalbuffer; 
	glGenBuffers(1, &normalbuffer); 
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer); 
	glBufferData(GL_ARRAY_BUFFER,sizeof(normals_data), &normals_data, GL_STATIC_DRAW);

	GLuint UVbuffer;
	glGenBuffers(1, &UVbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, UVbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data, GL_STATIC_DRAW);



	//////////////////////////////////////////]
	loadTexture("textures/fire.jpg",1000,1000,4,1);
	
	textures_array_id[1] = glGetUniformLocation(programID, "myTextureSampler");

	GLuint vertexbuffersphere;
	glGenBuffers(1, &vertexbuffersphere);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffersphere);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
	

	GLuint UVbufferSphere;
	glGenBuffers(1, &UVbufferSphere);
	glBindBuffer(GL_ARRAY_BUFFER,UVbufferSphere);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);


	/////////  NORMAL
	GLuint normalbufferSphere; 
	glGenBuffers(1, &normalbufferSphere); 
	glBindBuffer(GL_ARRAY_BUFFER, normalbufferSphere); 
	glBufferData(GL_ARRAY_BUFFER,normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
	////////
	//////////////////////////////////////////

	//lighting i hope
	glUseProgram(programID);
	GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
	GLfloat PLEASE = 0.7f;
	glUniform1f(power2, PLEASE);
	
	//musica
	//link : https://learn.microsoft.com/en-us/previous-versions/dd798676(v=vs.85)
	//https://community.khronos.org/t/playing-sound-in-opengl/58399
	//PlaySound(TEXT("explode.wav"), NULL, SND_FILENAME | SND_ASYNC);
	/*ISoundEngine* engine = createIrrKlangDevice();
	if (!engine)
    	return 0; // error starting up the engine
    engine->play2D("somefile.mp3", true);

	char i = 0;
	std::cin >> i; // wait for user to press some key

	ngine->drop(); // delete engine
	return 0;*/

	//lightingShader.use();
	//lightingShader.setVec3("LightPosition_worldspace", 1.0f, 0.5f, 0.31f);

	do {

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);
		//glUseProgram(programID2);

		//camera
		//glm::mat4 MVP = camera_function(&coords, &fov);


		//compute MVP matrix from keyboard and mouse
		computeMatricesFromInputs();
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		glm::mat4 ModelMatrix = glm::mat4(1.0);
		glm::mat4 MVP = ProjectionMatrix*ViewMatrix*ModelMatrix;
		//
		//glGenerateMipmap(GL_TEXTURE_2D);getViewMatrix
		// Send our transformation to the currently bound shader, // in the "MVP" uniform
		glUniformMatrix4fv (MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv (ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);
		

		glm::vec3 lightPos = glm::vec3(4,4,4);
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
		
		/////
		//glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

		//glUniform1i(TextureID, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Textures_array[0]);
		glUniform1i(textures_array_id[0],0);


		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 2nd attribute buffer : colors
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, UVbuffer);
		glVertexAttribPointer(
			1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);
/*
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glVertexAttribPointer(
			1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			2,                                // size
			GL_FLOAT,                         // type
			GL_TRUE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);
*/
		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, 19*19*2*3); // 3 indices starting at 0 -> 1 triangle

		// Swap buffers
		//glfwSwapBuffers(window);
		//glfwPollEvents();

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		//GLuint UVbufferSphere = loadTexture("textures/fire.jpg",1000,1000,4,0);

		//glUniform1i(TextureID, 0);
		//glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

		//glGenerateMipmap(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0+1);
		glBindTexture(GL_TEXTURE_2D, Textures_array[1]);
		glUniform1i(textures_array_id[1],1);
		// Load it into a VBO
		

		/*glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, UVbufferSphere);
		*/
		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffersphere);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 2nd attribute buffer : colors
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, UVbufferSphere);
		glVertexAttribPointer(
			1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		); 
/*
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, normalbufferSphere);
		glVertexAttribPointer(
			1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			2,                                // size
			GL_FLOAT,                         // type
			GL_TRUE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		); */
		
		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0,  vertices.size()); // 3 indices starting at 0 -> 1 triangle

		
		// Debug things :)
		//if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) // DEBUG
		//	printf("x: %f, y: %f z: %f, fov: %f\n", coords.x, coords.y, coords.z, fov);


		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

		
	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_SPACE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	// Cleanup VBO
	
	glDeleteProgram(programID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;

}

