#ifndef _LIBSLOADED
#define _LIBSLOADED 1
#include <vector>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#endif

using namespace glm;
using namespace std;

class Crater {       // The class
  public:             // Access specifier
    std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> uvs;
	GLuint vertexbuffer;
	GLuint uvbuffer;
	glm::vec2 position;
	float scale = 3;
    
    void initCrater(std::vector<glm::vec3> verticesO,
		std::vector<glm::vec3> normalsO,
		std::vector<glm::vec2> uvsO)
    {
		vertices.resize(verticesO.size());
		normals.resize(normalsO.size());
		uvs.resize(uvsO.size());

		copyVector(0, verticesO);
		copyVector(1, normalsO);
		copyVector(uvsO);
		// Parse the sphere.obj file
		resizeCrater(1.75f);
		setFixedZ(0.01f);

		glGenBuffers(1, &vertexbuffer); // crater location currently unknown. Vertices WILL change.
		glGenBuffers(1, &uvbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
    }

	// Accessors for the crater's fields
	std::vector<glm::vec3> getVertices() { return vertices;		 }
	std::vector<glm::vec3> getNormals()  { return normals;		 }
	std::vector<glm::vec2> getUvs()		  { return uvs; 		   }
	glm::vec2 getPosition() 				{ return position;		}
	GLuint getVertexbuffer()				{ return vertexbuffer; }
	GLuint getUvbuffer()					{ return uvbuffer;		}
	float getScale()						 { return scale;		 }

	void setFixedZ(float height)
	{
		for (int i = 0; i < (vertices).size(); ++i)
		{
			(vertices)[i].z = height;
		}
	}

	void copyVector(const int mode, std::vector<glm::vec3> from)
	{
		switch(mode) {
		case 0:
			for (int i = 0; i < from.size(); i++)
			{
				vertices[i].x = from[i].x;
				vertices[i].y = from[i].y;
				vertices[i].z = from[i].z;
			}
		case 1:
			for (int i = 0; i < from.size(); i++)
			{
				normals[i].x = from[i].x;
				normals[i].y = from[i].y;
				normals[i].z = from[i].z;
			}
		}
		
	}

	void copyVector(std::vector<glm::vec2> from)
	{
		for (int i = 0; i < from.size(); i++)
		{
			uvs[i].x = from[i].x;
			uvs[i].y = from[i].y;
		}
	}

	void parsePosition(glm::vec2 newPosition)
	{
		position.x = newPosition.x;
		position.y = newPosition.y;
		for (int i = 0; i < (vertices).size(); ++i)
		{
			(vertices)[i].x += newPosition.x;
			(vertices)[i].y += newPosition.y;
		}
	}

	void resizeCrater(float multiplier)
	{
        scale *= multiplier;
        glm::vec2 newPosition(position.x, position.y);
        newPosition *= multiplier;
		for (int i = 0; i < (vertices).size(); ++i)
		{
			// Return object to "starter" position, to avoid losing crater position
			(vertices)[i].x -= position.x;
			(vertices)[i].y -= position.y;
			// Rescale object
			(vertices)[i].x *= multiplier;
			(vertices)[i].y *= multiplier;
			// Move object to new position. Ensure the center stays the same
			(vertices)[i].x += newPosition.x;
			(vertices)[i].y += newPosition.y;
		}
        position = newPosition;
	}
};