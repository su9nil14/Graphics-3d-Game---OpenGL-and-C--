
#include "VCamera.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <windows.h>


VCamera::VCamera() {
	this->speed = 10.0f;
	this->position = glm::vec3(0.0f, 0.0f, 0.0f);
	this->cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	this->cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
}

glm::mat4 VCamera::getViewMatrix() {
							//eye		//center		/up
    return glm::lookAt(position, cameraFront+position, cameraUp);
}


void VCamera::mouseUpdate(float x, float y) {

	glm::vec3 x_axis = glm::cross(cameraFront, cameraUp);
	glm::quat pitch = glm::angleAxis(y, x_axis);
	glm::quat yaw = glm::angleAxis(x, cameraUp);
	glm::quat orientation =  yaw * pitch;

	glm::mat4 rotation = glm::mat4_cast(orientation);
	if (pitch.y > 99.0f) pitch.y = 99.0f;
	if (pitch.y < -99.0f) pitch.y = -99.0f;
	cameraFront = glm::mat3(rotation) * cameraFront;
}

bool VCamera::collisonWithWall(glm::vec3 location) {
    return location.x < 0.0f || location.x > 99.0f ||
           location.z < -99.0f || location.z > 99.0f ;
}

void VCamera::update(glm::vec3 new_position) {
	
    glm::vec3 new_posX = glm::vec3(new_position.x, position.y, position.z);
 
        if (!collisonWithWall(new_posX)) {
            //collided = false;
			position.x = new_position.x;
        }
 
    
    glm::vec3 new_posZ = glm::vec3(position.x, position.y, new_position.z);
    
        if (!collisonWithWall(new_posZ)) {
            //collided = false;
			position.z = new_position.z;
        }

} 
//delta is the ime elaspsed
void VCamera::moveForward(float delta) {
	glm::vec3 new_position = position + speed * delta * glm::vec3(cameraFront.x, 0.0f, cameraFront.z);
	update(new_position);
}

void VCamera::moveBackward(float delta) {
	glm::vec3 new_position = position - speed * delta * glm::vec3(cameraFront.x, 0.0f, cameraFront.z);
	update(new_position);
}

void VCamera::moveLeft(float delta) {
	glm::vec3 new_position = position + (speed * delta * glm::cross(cameraUp, cameraFront));
	update(new_position);

}

void VCamera::moveRight(float delta) {
	glm::vec3 new_position = position - (speed * delta * glm::cross(cameraUp, cameraFront));
	update(new_position);
	
}


