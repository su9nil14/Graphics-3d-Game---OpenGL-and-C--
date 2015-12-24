#include "VboObject.h"


GLuint loc1, loc2, loc3; //


VboObject::VboObject(){

	this->g_point_count = 0;
	this->vao_object = 0;
};

void VboObject::myObject(const char *filename, GLuint shaderProgramID) {

#pragma region MESH LOADING
	/*----------------------------------------------------------------------------
	MESH LOADING FUNCTION
	----------------------------------------------------------------------------*/
	const aiScene* scene = aiImportFile (filename, aiProcess_Triangulate); // TRIANGLES! | aiProcess_FlipUVs
	fprintf (stderr, "Reading mesh %s\n", filename);
	if (!scene) {
		fprintf (stderr, "ERROR: reading mesh %s\n", filename);
	}
	printf ("  %i animations\n", scene->mNumAnimations);
	printf ("  %i cameras\n", scene->mNumCameras);
	printf ("  %i lights\n", scene->mNumLights);
	printf ("  %i materials\n", scene->mNumMaterials);
	printf ("  %i meshes\n", scene->mNumMeshes);
	printf ("  %i textures\n", scene->mNumTextures);
	
	const aiMesh* mesh = scene->mMeshes[0];
	g_point_count  = mesh->mNumVertices;
	printf ("  %i vertices\n", g_point_count);
	/*===============================================================================================================================================
	COPY THE MESH INTO BUFFERS */


	//Note: you may get an error "vector subscript out of range" if you are using this code for a mesh that doesnt have positions and normals
	//Might be an idea to do a check for that before generating and binding the buffer.

	loc1 = glGetAttribLocation(shaderProgramID, "vertex_position");
	loc2 = glGetAttribLocation(shaderProgramID, "vertex_normal");
	loc3 = glGetAttribLocation(shaderProgramID, "vertex_texture");

	
	glGenVertexArrays(1, &vao_object);
	glBindVertexArray(vao_object);

	GLuint vbuffer; //same buffer rather than different one
	//vertex
	glGenBuffers (1, &vbuffer);
	glBindBuffer (GL_ARRAY_BUFFER, vbuffer);
	glBufferData (GL_ARRAY_BUFFER, g_point_count * 3 * sizeof (float), mesh->mVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray (loc1);
	glVertexAttribPointer (loc1, 3, GL_FLOAT, 0, 0, 0);

	GLuint nbuffer;
	//normals
	glGenBuffers (1, &nbuffer);
	glBindBuffer (GL_ARRAY_BUFFER, nbuffer);
	glBufferData (GL_ARRAY_BUFFER, g_point_count * 3 * sizeof (float), mesh->mNormals, GL_STATIC_DRAW);
	glEnableVertexAttribArray (loc2);
	glVertexAttribPointer (loc2, 3, GL_FLOAT, 0, 0, 0);

	// texture
	GLuint tbuffer;
	float *myTextureCoords = new float[g_point_count * 2]; 

	if (mesh->HasTextureCoords (0)) {
		for(int i = 0; i < g_point_count; i++) {
			const aiVector3D* vt = &(mesh->mTextureCoords[0][i]);
			myTextureCoords[i * 2] = (GLfloat)vt->x;
			myTextureCoords[i * 2 + 1] = (GLfloat)vt->y;
		}
	}
	glGenBuffers (1, &tbuffer);
	glBindBuffer (GL_ARRAY_BUFFER, tbuffer);
	glBufferData (GL_ARRAY_BUFFER, g_point_count * 2 * sizeof (float), myTextureCoords, GL_STATIC_DRAW);
	glEnableVertexAttribArray (loc3);
	glVertexAttribPointer (loc3, 2, GL_FLOAT, 0, 0, 0);
	free (myTextureCoords);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);	

	aiReleaseImport (scene);
	printf ("mesh loaded\n");
}

//#pragma endregion VBO_FUNCTIONS
#pragma endregion MESH LOADING 

//LOADING TEXTURES=================================================================================================================

#pragma region TEXTURE_FUNCTIONS
GLuint VboObject::loadTexture(const char * imagepath){

	GLuint tex;
	int x, y, n;
	int force_channels = 4;
	unsigned char* image_data = NULL;
	int width_in_bytes = -1;
	unsigned char *top = NULL;
	unsigned char *bottom = NULL;
	unsigned char temp = 0;
	int half_height = -1;
	
	printf ("loading texture from file: %s\n", imagepath);
	image_data = stbi_load (imagepath, &x, &y, &n, force_channels);
	if (!image_data) {
		fprintf (stderr, "ERROR: could not load %s\n", imagepath);
		return false;
	}
	// NPOT check
	if ((x & (x - 1)) != 0 || (y & (y - 1)) != 0) {
		fprintf (
			stderr, "WARNING: texture %s is not power-of-2 dimensions\n", imagepath
		);
	}
	width_in_bytes = x * 4;
	half_height = y / 2;

	for (int row = 0; row < half_height; row++) {
		top = image_data + row * width_in_bytes;
		bottom = image_data + (y - row - 1) * width_in_bytes;
		for (int col = 0; col < width_in_bytes; col++) {
			temp = *top;
			*top = *bottom;
			*bottom = temp;
			top++;
			bottom++;
		}
	}
	glGenTextures (1, &tex);
	glActiveTexture (GL_TEXTURE0);
	glBindTexture (GL_TEXTURE_2D, tex);
	glTexImage2D (GL_TEXTURE_2D,0,GL_RGBA,x,y,0,GL_RGBA,GL_UNSIGNED_BYTE,image_data);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	
	free(image_data); 
	return tex;
}
#pragma endregion TEXTURE_FUNCTIONS


void VboObject::draw() const {

	glBindVertexArray(vao_object);
	glDrawArrays(GL_TRIANGLES, 0, g_point_count);
	glBindVertexArray(0);
}

GLint VboObject::point_count() const {
	return g_point_count;
}

GLuint VboObject::object() const {
	return vao_object;
}
