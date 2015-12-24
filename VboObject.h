#pragma once

#include <GL/glew.h>
#include "maths_funcs.h" 
//glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
// Assimp includes
#include <assimp/cimport.h> // C importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <vector> // STL dynamic memory.
#include <random>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h" 


class VboObject{ 


public:
	VboObject();
	
	GLint g_point_count;
	GLuint vao_object;


    void myObject(const char *filename, GLuint shaderProgramID);

	GLuint loadTexture(const char *filename);
    
    void draw() const;
    
    GLint point_count() const;

	GLuint object() const;
    
};