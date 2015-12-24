
#pragma once

#define GLM_FORCE_RADIANS

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include "maths_funcs.h"


class VCamera {
public:
    VCamera();
    
    GLfloat speed;
    glm::vec3 position;
    glm::vec3 cameraFront;
    glm::vec3 cameraUp;
    
    virtual glm::mat4 getViewMatrix();

    virtual void mouseUpdate(float deltaX, float deltaY);
	bool collisonWithWall(glm::vec3 location);
    virtual void update(glm::vec3 new_position);
    virtual void moveForward(float delta);
    virtual void moveBackward(float delta);
    virtual void moveLeft(float delta);
    virtual void moveRight(float delta);

};