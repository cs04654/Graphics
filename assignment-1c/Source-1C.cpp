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
#include <fstream>
#include <algorithm>
#include <sstream>
#include <chrono>
#include <thread>
#include "stb_image.h"
#include "gridarray.h"

#ifndef _LIBSLOADED
#define _LIBSLOADED 1
// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#endif

#include "Crater.cpp"

using namespace glm;
using namespace std;

#define STEP 0.25
#define SPHEREGRAVITYSTEP 0.01
#define MAXSPHEREGRAVITY 0.5
#define MAXCRATERNUMBER 10
#define LIGHTINGFRAMEDURATION 50
float sphrereGravity = 0.1;
GLuint Textures_array[3];

Crater craters[MAXCRATERNUMBER];
int craterCounter = 0; // Combined with mod MAXCRATERNUMBER, we can ensure there won't be oversaturation

glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

glm::mat4 getViewMatrix() {
	return ViewMatrix;
}
glm::mat4 getProjectionMatrix() {
	return ProjectionMatrix;
}


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


glm::mat4 camera_function(glm::vec3* coords, float* fov)
{
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) // FORWARD
		if (coords->x <  200) // After this, object disappears
				coords->x += STEP;
	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) // BACKWARD
		if (coords->x > -200) // After this, object disappears
			coords->x -= STEP;

	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) // UPWARD
		if (coords->z <  1000) // After this, object disappears
			coords->z += STEP;
	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) // DOWNWARD
		if (coords->z > -1000) // After this, object disappears
			coords->z -= STEP;

	// E and C buttons are for keyboards without a numpad. Long live 60%
	if (glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) // ZOOM IN
		if (*fov > 5) // Prevents camera mirroring
			*fov -= 2*STEP;
	if (glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) // ZOOM OUT
		if (*fov < 150) // Prevents zooming out too much
			*fov += 2*STEP;

	// Camera things
	glm::mat4 Projection = glm::perspective(glm::radians(*fov), 4.0f / 4.0f, 0.1f, 1000.0f);
	glm::mat4 View = glm::lookAt(
        *coords, 
		glm::vec3(50.0f, 50.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f)
    );

    // Model matrix : an identity matrix (model will be at the origin)
	glm::mat4 Model = glm::mat4(1.0f);
	// Our ModelViewProjection : multiplication of our 3 matrices
	return Projection * View * Model; // Remember, matrix multiplication is the other way around
}

void playSound()
{
	system("aplay sounds/explosion.wav");
}

std::vector<glm::vec3> sphere_function(std::vector<glm::vec3> vertices, bool* visibleSphere, bool* collisionDetection)
{
	if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
		if (sphrereGravity < MAXSPHEREGRAVITY)
			sphrereGravity += SPHEREGRAVITYSTEP;
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
		if (sphrereGravity > 0.01f)
			sphrereGravity -= SPHEREGRAVITYSTEP;

	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && !*visibleSphere && vertices[0].z < 0.1) {

		/* Generate random sphere position. Since the crater has a 3 unit length/width,
		 * it spawns between 2 and 98. Random position has 0.1f accuracy :)
		 */
		float x = (rand() % 960 + 20)/10;
		float y = (rand() % 960 + 20)/10;
		
		/* REMINDER: When the sphere collides, it is not destroyed, just not rendered!!!
		 * That means, the sphere has it's old coordinates. Therefore, we need to calculate
		 * the x and y deltas, from a reference point (vertices[0]) to the new position. 
		 */
		float deltax = -vertices[0].x + x;
		float deltay = -vertices[0].y + y;

		// Apply the delta to all vertices
		for (int i = 0; i < vertices.size(); ++i)
		{
			vertices[i].x += deltax;
			vertices[i].y += deltay;
			vertices[i].z += 20.0f;
		}

		// Reset the appropriate flags
		*collisionDetection = false;
		*visibleSphere = true;

	} else if (*visibleSphere) {
		/* Decrease height for every frame. Not a fixed time interval,
		 * but the program is light enough to not affect anything
		 */
		for (int i = 0; i < vertices.size(); ++i)
		{
			vertices[i].z -= sphrereGravity;
		}
	}

	if (vertices[0].z < 0.1 && *visibleSphere) {
		thread sound(playSound); // Play boom boom sound
		*collisionDetection = true;
		*visibleSphere = false;
		sound.detach();
	}

	return vertices;
}

void drawObject(GLuint uniform, int textureOffset, GLuint vertexbuffer, GLuint uvbuffer, int drawCount)
{
	glActiveTexture(GL_TEXTURE0+textureOffset);
	glBindTexture(GL_TEXTURE_2D, Textures_array[textureOffset]);
	glUniform1i(uniform, textureOffset);

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
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glVertexAttribPointer(
		1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
		2,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	); 
	
	// Draw the triangle !
	glDrawArrays(GL_TRIANGLES, 0,  drawCount); // 3 indices starting at 0 -> 1 triangle
}

void renderCraters(GLuint uniform)
{
	// Determine how many craters will be rendered
	int numberOfCraters = craterCounter;
	if (craterCounter > MAXCRATERNUMBER) {
		numberOfCraters = MAXCRATERNUMBER;
	}

	for (int i = 0; i < numberOfCraters; i++)
	{
		Crater crater = craters[i];
		std::vector<glm::vec3> vertices = crater.getVertices();
		std::vector<glm::vec3> normals = crater.getNormals();
		std::vector<glm::vec2> uvs = crater.getUvs();
		GLuint vertexbuffer = crater.getVertexbuffer();
		GLuint uvbuffer = crater.getUvbuffer();

		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

		drawObject(uniform, 2, vertexbuffer, uvbuffer, vertices.size());
	}
	
}

/// @brief  Checks there is a crater in sphere landsite. If there is, it enlargens the crater
/// @param  position 
/// @return Whether a new crater should be created.
bool checkCrater(glm::vec2 position)
{
	// Determine how many craters will be rendered
	int numberOfCraters = craterCounter;
	if (craterCounter > MAXCRATERNUMBER) {
		numberOfCraters = MAXCRATERNUMBER;
	}

	for (int i = 0; i < numberOfCraters; i++)
	{
		Crater crater = craters[i];
		glm::vec2 craterPos = crater.getPosition();

		// Determine space the crate takes up. Janky maths, no time :p
		float maxX = craterPos.x;
		float minX = craterPos.x + crater.scale;
		float maxY = craterPos.y;
		float minY = craterPos.y - crater.scale;

		// Check if sphere is withing current crater's range
		if (position.x > maxX && position.x > minX && position.y < maxY && position.y > minY)
		{
			craters[i].resizeCrater(1.25f);
			return false;
		}
	}
	return true;
}

bool lightingEffect(int* lightingFrameCounter, float* brightness)
{
	(*lightingFrameCounter)++;
	int currentFrameCycle = *lightingFrameCounter % 30;
	if (currentFrameCycle < 20) {
		*brightness = 2.0f;
	} else {
		*brightness = 1.5f;
	}
	if (*lightingFrameCounter >= LIGHTINGFRAMEDURATION) {
		*brightness = 1.0f;
		return false;
	}
	return true;
}

int main(void)
{
	glEnable ( GL_LIGHTING ) ; // enabling lighting changed
	glEnable ( GL_COLOR_MATERIAL ) ; // enabling color changed
	glGenTextures(3,Textures_array);

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
	//GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	//GLuint MatrixID2 = glGetUniformLocation(programID, "MVP2");

	GLuint programID = LoadShaders("StandardShading.vertexshader","StandardShading.fragmentshader");
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	GLuint ModelMatrixID = glGetUniformLocation(programID, "M");
	GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
	GLuint MVP = glGetUniformLocation(programID, "MV");
	GLuint power2 = glGetUniformLocation(programID, "power");
	
	// Initial camera position
	glm::vec3 coords = glm::vec3(50.0f, -70.0f, 75.0f);

	srand(time(NULL));
	float fov = 45.0f;

	// Load and parse the grass, fire and crater textures
	GLuint uniform = glGetUniformLocation(programID, "myTextureSampler"); // Used to be an array (textures_array_id) for.. some reason
	loadTexture("textures/ground2.jpg", 715, 715, 4, 0);
	loadTexture("textures/fire.jpg", 1000, 1000, 4, 1);
	loadTexture("textures/crater1.jpg", 225, 225, 4, 2);

	// Calculate plane vertices
	GLuint vertexbuffer;
	GLfloat g_vertex_buffer_data[3*3*2*19*19];
	calculatePlaneBufferData(g_vertex_buffer_data);
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	GLuint UVbuffer;
	glGenBuffers(1, &UVbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, UVbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data, GL_STATIC_DRAW);

	// Load the sphere.obj file
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> uvs;
	loadOBJ("Obj files/ball.obj", vertices, uvs, normals);
	vertices = resizeObject(1.75f, vertices, vertices.size());

	GLuint vertexbuffersphere;
	glGenBuffers(1, &vertexbuffersphere);
	/* Sphere vertex buffer data is bound right before the Sphere is drawn,
	 * as the vertices vector constantly changes. No need to bind it here.
	 */ 
	
	GLuint UVbufferSphere;
	glGenBuffers(1, &UVbufferSphere);
	glBindBuffer(GL_ARRAY_BUFFER,UVbufferSphere);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

	bool visibleSphere = false;
	
	// Load the crater.obj file, which we'll use as default values
	std::vector<glm::vec3> craterVertices;
	std::vector<glm::vec3> craterNormals;
	std::vector<glm::vec2> craterUvs;
	loadOBJ("Obj files/crater1.obj", craterVertices, craterUvs, craterNormals);

	//lighting i hope
	glUseProgram(programID);
	GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

	// Lighting effect
	bool lightingEffectActive = false;
	int lightingFrameCounter = 0;
	GLfloat brightness = 1.0f;

	do {

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);
		glUniform1f(power2, brightness);

		// Calculate the camera MVP
		glm::mat4 MVP = camera_function(&coords, &fov);
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

		// Calculate light position
		glm::vec3 lightPos = glm::vec3(4,4,4);
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

		// Calculate the new sphere position
		bool collision = false; // If collision is detected, add crater
		vertices = sphere_function(vertices, &visibleSphere, &collision);

		// Draw our plane
		drawObject(uniform, 0, vertexbuffer, UVbuffer, 19*19*2*3);
		
		// Draw our sphere, if it hasn't collided
		if (visibleSphere) {
			// Vertices need to be re-binded to the buffer
			glBindBuffer(GL_ARRAY_BUFFER, vertexbuffersphere);
			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
			// Draw our sphere
			drawObject(uniform, 1, vertexbuffersphere, UVbufferSphere, vertices.size());
		}

		if (lightingEffectActive) // Lighting effect active
			lightingEffectActive = lightingEffect(&lightingFrameCounter, &brightness);


		if (collision) {
			glm::vec2 crashPos(vertices[0].x, vertices[0].y);
			// check if there is a sphere in a 3x3 square, if there isn't one, create it
			if (checkCrater(crashPos)){
				crashPos += glm::vec2(1.5f, -1.5f); // Start of crater texture is half of the crater's scale in each dir
				craters[craterCounter % MAXCRATERNUMBER].initCrater(craterVertices, craterNormals, craterUvs);
				craters[craterCounter % MAXCRATERNUMBER].parsePosition(crashPos);
				craterCounter++;
			}
			// Activate lighting effect
			lightingEffectActive = true;
			lightingFrameCounter = 0;
			//brightness = 2.0f;
		}

		renderCraters(uniform);
		
		// Debug things :)
		if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) // DEBUG
			printf("Camera  | x: %f, y: %f z: %f, fov: %f\n"
					"Sph_ref | x: %f, y: %f z: %f\n\n",
					coords.x, coords.y, coords.z, fov,
					vertices[0].x, vertices[0].y, vertices[0].z);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

		
	} // Check if the SPACE key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_SPACE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	// Cleanup VBO
	
	glDeleteProgram(programID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;

}

